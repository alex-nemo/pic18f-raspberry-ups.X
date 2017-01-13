#include <xc.h>
#include "pid.h"
#include "test.h"

/**
 * Bits de configuration:
 */

// Oscillateur interne:
#pragma config OSC = INTIO2     // Internal RC oscillator, port function on RA6 and port function on RA7
#pragma config IESO = OFF       // Internal External Switchover mode enabled.
#pragma config FSCM = OFF       // Fail-Safe Clock Monitor disabled

// Nécessaires pour ICSP / ICD:
#pragma config DEBUG = ON       // Background debugger enabled, RB6 and RB7 are dedicated to In-Circuit Debug.
#pragma config MCLRE = ON       // MCLR pin enabled, RA5 input pin disabled
#pragma config WDT = OFF        // Watchdog inactif.
#pragma config LVP = OFF        // Low-Voltage ICSP disabled.

#ifndef TEST

void interrupt low_priority bassePriorite() {
    if (INTCONbits.T0IF) {
        INTCONbits.T0IF = 0;
        TMR0H = 0xFF;
        TMR0L = 0x37;

        ADCON0bits.GODONE = 1;
    }
    
    if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        if (ADRESH < 158) {
            PORTAbits.RA0 = 1;
            TRISBbits.RB3 = 0;
        }
        if (ADRESH > 200) {
            PORTAbits.RA0 = 0;
            TRISBbits.RB3 = 1;            
        }
    }
}

/**
 * Initialise le hardware.
 */
static void hardwareInitialise() {
    // Horloge à 8MHz:
    OSCCONbits.IRCF = 7;    // 7 ==> 8MHz
    
    // Configuration des ports d'entrée / sortie
    TRISA = 0b00000000;
    TRISB = 0b00011011;
    
    // Charge l'accumulateur secondaire
    TRISBbits.RB2 = 0;
    PORTBbits.RB2 = 1;
    
    TRISAbits.RA7 = 1;
    PORTAbits.RA7 = 1;
    
    // PWM à 200kHz
    CCP1CONbits.CCP1M = 12; // PWM actif, P1A actif haut.
    CCP1CONbits.P1M = 0;    // Sortie uniquement P1A (RB3) 
    PR2 = 40;               // Période de 40
    CCPR1L = 8;             // Cycle de travail de 20%
    T2CONbits.T2CKPS = 0;
    T2CONbits.TMR2ON = 1;
    
    
    // Active le temporisateur 0 pour surveiller L'accumulateur principal:
    // Période d'interruption: 100uS
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 1;
    T0CONbits.TMR0ON = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    INTCON2bits.TMR0IP = 0;
    
    // Configure le convertisseur analogique pour un temps de conversion de 24uS
    ADCON0bits.ADON = 1;
    ADCON0bits.CHS = 6;
    ADCON1 = 0b00101111;    // Active AN6 et AN4 comme entrées analogiques.
    ADCON2bits.ADFM = 0;    // Justification à gauche.
    ADCON2bits.ADCS = 5;    // Horloge de conversion: Fosc / 8
    ADCON2bits.ACQT = 5;    // Conversion: 12 TAD.
    
    PIE1bits.ADIE = 1;      // Interruptions du module A/D
    IPR1bits.ADIP = 0;      // Basse priorité.
    
    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void main(void) {
    hardwareInitialise();
    while(1);
}
#endif

#ifdef TEST
void main(void) {
    initialiseTests();
    testPid();
    finaliseTests();
    while(1);
}
#endif
