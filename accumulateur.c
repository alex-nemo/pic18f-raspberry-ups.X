#include "accumulateur.h"
#include "test.h"

/**
 * État actuel de l'accumulateur.
 */
Accumulateur accumulateur = {0, 0, 0};

void initialiseAccumulateur() {
    accumulateur.accumulateurDisponible = 0;
    accumulateur.accumulateurEnCharge = 0;
    accumulateur.accumulateurPresent = 0;
}

Accumulateur *machineAccumulateur(unsigned char vacc) {
    // Si la tension de sortie de l'accumulateur est inférieur à 2V 
    // ou supérieure à 4.5V, alors l'accumulateur n'est pas présent ou
    // n'est pas utilisable.
    if ( (vacc < 51) || (vacc > 114) ) {
        accumulateur.accumulateurDisponible = 0;
        accumulateur.accumulateurEnCharge = 0;
        accumulateur.accumulateurPresent = 0;
        return &accumulateur;
    }

    accumulateur.accumulateurPresent = 1;
    
    if (accumulateur.accumulateurEnCharge) {
        // Si la tension de sortie de l'accumulateur est supérieure à 4.2,
        // il est temps d'arrêter la charge:
        if (vacc > 107) {
            accumulateur.accumulateurEnCharge = 0;
        }
    } else {
        // Si la tension de sortie de l'accumulateur est inférieur à 3.6V, 
        // il est temps de le mettre en charge:
        if (vacc < 92) {
            accumulateur.accumulateurEnCharge = 1;
        }
    }
    
    // Si la tension de sortie de l'accumulateur est supérieure à 3.2V,
    // on considère que l'accumulateur est utilisable:
    if (vacc > 82) {
        accumulateur.accumulateurDisponible = 1;
    } else {
        accumulateur.accumulateurDisponible = 0;
    }

    return &accumulateur;
}

#ifdef TEST

void testeAccumulateurDetectePresence() {
    initialiseAccumulateur();
    verifieEgalite("ACCPR1", machineAccumulateur( 50)->accumulateurPresent, 0);
    verifieEgalite("ACCPR2", machineAccumulateur( 51)->accumulateurPresent, 1);
    verifieEgalite("ACCPR3", machineAccumulateur(114)->accumulateurPresent, 1);
    verifieEgalite("ACCPR4", machineAccumulateur(115)->accumulateurPresent, 0);
}

void testeAccumulateurCycleDeCharge() {
    initialiseAccumulateur();
    
    verifieEgalite("ACCCY01", machineAccumulateur(107)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY02", machineAccumulateur(100)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY03", machineAccumulateur( 92)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY04", machineAccumulateur( 91)->accumulateurEnCharge, 1);
    verifieEgalite("ACCCY05", machineAccumulateur(100)->accumulateurEnCharge, 1);
    verifieEgalite("ACCCY06", machineAccumulateur(107)->accumulateurEnCharge, 1);
    verifieEgalite("ACCCY07", machineAccumulateur(108)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY08", machineAccumulateur(107)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY09", machineAccumulateur(100)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY10", machineAccumulateur( 92)->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY11", machineAccumulateur( 91)->accumulateurEnCharge, 1);    
}

void testeAccumulateurDetecteDisponibilite() {
    initialiseAccumulateur();
    
    verifieEgalite("ACCDI01", machineAccumulateur( 82)->accumulateurDisponible, 0);
    verifieEgalite("ACCDI01", machineAccumulateur( 83)->accumulateurDisponible, 1);
    verifieEgalite("ACCDI01", machineAccumulateur(107)->accumulateurDisponible, 1);
    verifieEgalite("ACCDI01", machineAccumulateur(108)->accumulateurDisponible, 1);
}

void testeAccumulateur() {
    testeAccumulateurDetectePresence();
    testeAccumulateurCycleDeCharge();
    testeAccumulateurDetecteDisponibilite();
}
#endif