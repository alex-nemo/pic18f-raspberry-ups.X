#include <xc.h>
#include "pid.h"
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
 * Initialise le hardware.
 */
static void hardwareInitialise() {
    // Horloge à 8MHz:
    OSCCONbits.IRCF = 4;    // 6 ==> 8MHz
    OSCTUNEbits.PLLEN = 0;

    // Prépare Temporisateur 1 pour 50 interruptions / s
    //            8 * 10^6                           8 * 10^6
    //  50 = ------------------- ==> T1CKPS * TMR1 = --------- = 40000
    //        4 * T1CKPS * TMR1                       4 * 50
    T1CONbits.TMR1CS = 0;       // Source: FOSC / 4
    T1CONbits.T1CKPS = 0;       // Pas de diviseur de fréquence.
    T1CONbits.T1RD16 = 1;       // Compteur de 16 bits.
    T1CONbits.TMR1ON = 1;       // Active le temporisateur.
    TMR1 = 25535;

    PIE1bits.TMR1IE = 1;        // Active les interruptions ...
    IPR1bits.TMR1IP = 0;        // ... de basse priorité ...
    PIR1bits.TMR1IF = 0;        // ... pour le temporisateur 1.
    
    // Prépare Temporisateur 2 pour un PWM de 50KHz:
    T2CONbits.T2CKPS = 0;       // Pas de diviseur de fréquence
    T2CONbits.T2OUTPS = 0b1001; // Diviseur de fréquence de sortie à 1:10
    T2CONbits.TMR2ON = 1;       // Active le temporisateur.

    // Configure PWM 1 pour émettre un signal de 1KHz:
    ANSELCbits.ANSC2 = 0;
    TRISCbits.RC2 = 0;
    
    CCP1CONbits.P1M = 0;        // Simple PWM sur la sortie P1A
    CCP1CONbits.CCP1M = 12;     // Active le CCP1 comme PWM.
    CCPTMRS0bits.C1TSEL = 0;    // Branche le CCP1 sur le temporisateur 2.
    PR2 = 80;                  // Largeur de pulsation: 1ms.
    CCPR1L = 0;                // Cycle de travail fixe.

    // Configure RC0 et RC1 pour gérer la direction du moteur:
    TRISCbits.RC0 = 0;
    TRISCbits.RC1 = 0;
    
    // Active le module de conversion A/D:
    TRISAbits.RA1 = 1;      // Active RA1 comme entrée.
    ANSELAbits.ANSA1 = 1;   // Active RA1/AN1 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 1;     // Branche le convertisseur sur AN1
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.

    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée des interruptions.
 */
void low_priority interrupt interruptionsBassePriorite() {
    static PID pid1 = {9, 0, 3, 0, 0, 0, 0};
    unsigned char pwm;

    if (PIR1bits.TMR1IF) {
        TMR1 = 60000;
        ADCON0bits.GO = 1;
        PIR1bits.TMR1IF = 0;
    }
    
    if (PIR1bits.ADIF) {
        pwm = calculatePID(&pid1, ADRESH, 153);
        CCPR1L = 60;
        PIR1bits.ADIF = 0;
    }
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
