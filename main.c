#include <xc.h>
#include "i2c.h"
#include "file.h"
#include "energie.h"
#include "test.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.

#ifndef TEST

/**
 * Maintient l'alimentation en gardant ouvert le transistor
 * d'entrée.
 */
void maintientAlimentation() {
    TRISAbits.RA5 = 0;
    PORTAbits.RA5 = 1;
}

/**
 * Coupe l'alimentation en fermant le transistor d'entrée.
 * Après ceci, le micro-contrôleur n'est plus alimenté.
 */
void coupeAlimentation() {
    TRISAbits.RA5 = 1;
}

/**
 * Configure le circuit selon l'état de l'accumulateur.
 * @param accumulateur L'état de l'accumulateur.
 */
void configureCircuit(Energie *energie) {

    // JAUNE: Si l'accumulateur n'est pas disponible, ou si il est en charge:
    if ( (!energie->accumulateurDisponible) || (energie->chargerAccumulateur)) {
        PORTCbits.RC1 = 1;
    } else {
        PORTCbits.RC1 = 0;        
    }
    PORTAbits.RA6 = energie->chargerAccumulateur;
    
    // VERT: Si l'accumulateur est disponible:
    PORTCbits.RC0 = energie->accumulateurDisponible;
    
    // ROUGE: Si l'accumulateur est sollicité:
    PORTCbits.RC5 = energie->solliciterAccumulateur;
    
    // Convertisseur BOOST: pour solliciter l'accumulateur:
    TRISCbits.RC2 = ~energie->solliciterAccumulateur;
    
    // Isoler l'accumulateur:
    if (energie->isolerAccumulateur) {
        coupeAlimentation();
    }
}

/**
 * Énumère les sources de conversion Analogique/Digital.
 * Intègre le numéro de canal analogique (AN1... AN6).
 */
typedef enum {
    ALIMENTATION = 0,
    BOOST = 1,
    ACCUMULATEUR = 2
} SourceAD;

/**
 * Gère les interruptions de basse priorité.
 */
void interrupt low_priority bassePriorite() {
    static SourceAD sourceAD = ACCUMULATEUR;
    Energie *energie;
    unsigned char conversion;

    // Lance une conversion Analogique / Digitale:
    if (INTCONbits.T0IF) {
        INTCONbits.T0IF = 0;
        TMR0H = 0xFF;
        TMR0L = 0x37;

        ADCON0bits.CHS = sourceAD;
        ADCON0bits.GODONE = 1;
    }
    
    // Reçoit le résultat de la conversion Analogique / Digitale.
    if (PIR1bits.ADIF) {
        conversion = ADRESH;
        PIR1bits.ADIF = 0;
        switch (sourceAD) {
            case ACCUMULATEUR:
                i2cExposeValeur(2, conversion);
                energie = mesureAccumulateur(conversion);
                sourceAD = BOOST;
                break;
                
            case BOOST:
                i2cExposeValeur(1, conversion);
                energie = mesureBoost(conversion);
                sourceAD = ALIMENTATION;
                break;

            case ALIMENTATION:
            default:
                i2cExposeValeur(0, conversion);
                energie = mesureAlimentation(conversion);
                sourceAD = ACCUMULATEUR;
                break;
        }
        configureCircuit(energie);
    }
}

/**
 * Initialise le hardware.
 */
static void hardwareInitialise() {
    // Horloge à 8MHz:
    OSCCONbits.IRCF = 6;    // 6 ==> 8MHz
    
    // Entrées analogiques:
    ANSELA = 0b00000111;
    ANSELB = 0;
    ANSELC = 0;
    
    // Configure le convertisseur analogique pour un temps de conversion de 24uS
    ADCON2bits.ADFM = 0;    // Justification à gauche.
    ADCON2bits.ADCS = 5;    // Horloge de conversion: Fosc / 8
    ADCON2bits.ACQT = 5;    // Conversion: 12 TAD.
    ADCON0bits.ADON = 1;    // Active le convertisseur.
    
    PIE1bits.ADIE = 1;      // Interruptions du module A/D
    IPR1bits.ADIP = 0;      // Basse priorité.

    // Entrées digitales:
    TRISA = 0b00011111;
    TRISB = 0b11111111;
    TRISC = 0b11011000;

    // Désactive la charge de l'accumulateur:
    PORTAbits.RA6 = 0;
    PORTAbits.RA7 = 0;
    
    // PWM à 200kHz
    TRISBbits.RB3 = 1;      // Bloque la sortie du PWM.
    CCP1CONbits.CCP1M = 12; // PWM actif, P1A actif haut.
    CCP1CONbits.P1M = 0;    // Sortie uniquement P1A (RC2) 
    PR2 = 40;               // Période de 40
    CCPR1L = 20;            // Cycle de travail de 50%
    T2CONbits.T2CKPS = 0;   
    T2CONbits.TMR2ON = 1;
    
    // Active le temporisateur 0 pour surveiller les entrées analogiques:
    // Période d'interruption: 100uS
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 1;
    T0CONbits.TMR0ON = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    INTCON2bits.TMR0IP = 0;

    // Active le MSSP1 en mode Esclave I2C:
    SSP1CON1bits.SSPEN = 1;             // Active le module SSP.    
    
    SSP1ADD = LECTURE_ALIMENTATION;     // Adresse de l'esclave.
    SSP1MSK = I2C_MASQUE_ADRESSES_ESCLAVES;
    SSP1CON1bits.SSPM = 0b1110;         // SSP1 en mode esclave I2C avec adresse de 7 bits et interruptions STOP et START.
        
    SSP1CON3bits.PCIE = 0;              // Désactive l'interruption en cas STOP.
    SSP1CON3bits.SCIE = 0;              // Désactive l'interruption en cas de START.
    SSP1CON3bits.SBCDE = 0;             // Désactive l'interruption en cas de collision.

    PIE1bits.SSP1IE = 1;                // Interruption en cas de transmission I2C...
    IPR1bits.SSP1IP = 0;                // ... de basse priorité.
    
    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void main(void) {
    maintientAlimentation();
    hardwareInitialise();
    while(1);
}
#endif

#ifdef TEST
void main(void) {
    initialiseTests();
    testeEnergie();
    testeFile();
    finaliseTests();
    while(1);
}
#endif
