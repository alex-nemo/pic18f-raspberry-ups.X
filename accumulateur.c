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
    accumulateur.accumulateurSollicite = 0;
}

Accumulateur *mesureAlimentation(unsigned char valim) {
    // Si l'alimentation tombe en dessous du 6V, on
    // active l'accumulateur secondaire.
    if (valim < 153) {
        // Mais seulement si l'accumulateur secondaire est présent et disponible.
        if (accumulateur.accumulateurPresent && accumulateur.accumulateurDisponible) {
            accumulateur.accumulateurEnCharge = 0;
            accumulateur.accumulateurSollicite = 1;
        }
    }
    
    // Si l'alimentation monte au dessus de 7V, on
    // arrête de solliciter l'accumulateur secondaire.
    if (valim > 178) {
        accumulateur.accumulateurSollicite = 0;
    }
    
    return &accumulateur;
}

Accumulateur *mesureAccumulateur(unsigned char vacc) {
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
        if (vacc < 91) {
            accumulateur.accumulateurEnCharge = 1;
        }
    }
    
    // Si la tension de sortie de l'accumulateur est supérieure à 3.2V,
    // on considère que l'accumulateur est utilisable:
    if (vacc > 80) {
        accumulateur.accumulateurDisponible = 1;
    } else {
        accumulateur.accumulateurDisponible = 0;
        accumulateur.accumulateurSollicite = 0;
    }

    return &accumulateur;
}

#ifdef TEST

// Conversion d'une tension x10 à la sortie d'un diviseur
// de tension (1/2) et convertie à 8 bits avec une référence de 5V.
// Exemple: 5 Volts ==> 50 ==> 127
#define CONVERSION_8BITS(x) ((x * 255)/100)

static void peut_detecter_que_l_accumulateur_est_present() {
    initialiseAccumulateur();
    verifieEgalite("ACCPR1", mesureAccumulateur(CONVERSION_8BITS(19))->accumulateurPresent, 0);
    verifieEgalite("ACCPR2", mesureAccumulateur(CONVERSION_8BITS(20))->accumulateurPresent, 1);
    verifieEgalite("ACCPR3", mesureAccumulateur(CONVERSION_8BITS(45))->accumulateurPresent, 1);
    verifieEgalite("ACCPR4", mesureAccumulateur(CONVERSION_8BITS(46))->accumulateurPresent, 0);
}

static void peut_completer_un_cycle_de_charge() {
    initialiseAccumulateur();
    
    verifieEgalite("ACCCY01", mesureAccumulateur(CONVERSION_8BITS(42))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY02", mesureAccumulateur(CONVERSION_8BITS(39))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY03", mesureAccumulateur(CONVERSION_8BITS(36))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY04", mesureAccumulateur(CONVERSION_8BITS(35))->accumulateurEnCharge, 1);
    verifieEgalite("ACCCY05", mesureAccumulateur(CONVERSION_8BITS(39))->accumulateurEnCharge, 1);
    verifieEgalite("ACCCY06", mesureAccumulateur(CONVERSION_8BITS(42))->accumulateurEnCharge, 1);
    verifieEgalite("ACCCY07", mesureAccumulateur(CONVERSION_8BITS(43))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY08", mesureAccumulateur(CONVERSION_8BITS(42))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY09", mesureAccumulateur(CONVERSION_8BITS(38))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY10", mesureAccumulateur(CONVERSION_8BITS(36))->accumulateurEnCharge, 0);
    verifieEgalite("ACCCY11", mesureAccumulateur(CONVERSION_8BITS(35))->accumulateurEnCharge, 1);    
}

static void peut_detecter_que_l_accumulateur_est_disponible() {
    initialiseAccumulateur();
    
    verifieEgalite("ACCDI01", mesureAccumulateur(CONVERSION_8BITS(31))->accumulateurDisponible, 0);
    verifieEgalite("ACCDI02", mesureAccumulateur(CONVERSION_8BITS(32))->accumulateurDisponible, 1);
    verifieEgalite("ACCDI03", mesureAccumulateur(CONVERSION_8BITS(42))->accumulateurDisponible, 1);
    verifieEgalite("ACCDI04", mesureAccumulateur(CONVERSION_8BITS(43))->accumulateurDisponible, 1);
}

static void sollicite_l_accumulateur_si_l_alimentation_fait_defaut() {
    initialiseAccumulateur();
    mesureAccumulateur(CONVERSION_8BITS(40));
    verifieEgalite("ACCSOL01", mesureAlimentation(CONVERSION_8BITS(60))->accumulateurSollicite, 0);
    verifieEgalite("ACCSOL02", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 1);
    verifieEgalite("ACCSOL03", mesureAlimentation(CONVERSION_8BITS(60))->accumulateurSollicite, 1);
    verifieEgalite("ACCSOL04", mesureAlimentation(CONVERSION_8BITS(70))->accumulateurSollicite, 1);
    verifieEgalite("ACCSOL04", mesureAlimentation(CONVERSION_8BITS(71))->accumulateurSollicite, 0);
    verifieEgalite("ACCSOL05", mesureAlimentation(CONVERSION_8BITS(70))->accumulateurSollicite, 0);
}

static void ne_solicite_plus_l_accumulateur_si_il_est_pas_disponible() {
    initialiseAccumulateur();
    
    mesureAccumulateur(CONVERSION_8BITS(41));
    verifieEgalite("ACCSL01", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 1);
    
    mesureAccumulateur(CONVERSION_8BITS(32));
    verifieEgalite("ACCSL02", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 1);

    mesureAccumulateur(CONVERSION_8BITS(31));
    verifieEgalite("ACCSL03", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 0);

    mesureAccumulateur(CONVERSION_8BITS(32));
    verifieEgalite("ACCSL04", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 1);
}

static void ne_solicite_pas_l_accumulateur_si_il_est_pas_disponible() {
    initialiseAccumulateur();

    mesureAccumulateur(CONVERSION_8BITS(31));
    verifieEgalite("ACCSP01", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 0);
    
}

static void ne_solicite_pas_l_accumulateur_si_il_est_pas_present() {
    initialiseAccumulateur();

    mesureAccumulateur(CONVERSION_8BITS(19));
    verifieEgalite("ACCSP01", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 0);

    mesureAccumulateur(CONVERSION_8BITS(19));
    verifieEgalite("ACCSP01", mesureAlimentation(CONVERSION_8BITS(59))->accumulateurSollicite, 0);
    
}

void testeAccumulateur() {
    peut_detecter_que_l_accumulateur_est_present();
    peut_detecter_que_l_accumulateur_est_disponible();
    peut_completer_un_cycle_de_charge();
    
    sollicite_l_accumulateur_si_l_alimentation_fait_defaut();
 
    ne_solicite_plus_l_accumulateur_si_il_est_pas_disponible();
    ne_solicite_pas_l_accumulateur_si_il_est_pas_disponible();
    ne_solicite_pas_l_accumulateur_si_il_est_pas_present();
}

#endif