#include "energie.h"
#include "test.h"

/** 
 * Énumère les états de l'accumulateur.
 */
typedef enum {
    /**
     * L'accumulateur est absent.
     */
    ABSENT,
    /**
     * L'accumulateur est présent, mais pas utilisable.
     */
    PAS_UTILISABLE,
    /**
     * L'accumulateur est utilisable, mais son niveau de charge est faible.
     */
    UTILISABLE_MAIS_FAIBLE,
    /**
     * L'accumulateur est utilisable.
     */
    UTILISABLE
} EtatAccumulateur;


/**
 * Énumère les états de la source d'alimentation.
 */
typedef enum {
    /**
     * L'alimentation est utilisable.
     */
    PRESENTE,
    /**
     * L'alimentation n'est pas utilisable.
     */
    DEFAILLANTE
} EtatAlimentation;


/**
 * Énumère les états du raspberry.
 */
typedef enum {
    /**
     * Le raspberry est probablement allumé, et a besoin de courant.
     */
    PROBABLEMENT_ACTIF,
    /**
     * Le raspberry ne consomme actuellement pas de courant.
     */
    INACTIF
} EtatRaspberry;

/** État actuel de l'accumulateur. */
static EtatAccumulateur etatAccumulateur = UTILISABLE;

/** État actuel de l'alimentation. */
static EtatAlimentation etatAlimentation = PRESENTE;

/** État actuel du raspberry. */
static EtatRaspberry etatRaspberry = PROBABLEMENT_ACTIF;

/**
 * Initialise les états internes.
 */
void initialiseEnergie() {
    etatAccumulateur = UTILISABLE;
    etatAlimentation = PRESENTE;
    etatRaspberry = PROBABLEMENT_ACTIF;
}

Energie energie;

/**
 * Calcule la configuration de l'accumulateur, selon les états de l'alimentation,
 * l'accumulateur et le raspberry.
 * @return La configuration de l'accumulateur.
 */
static Energie *etatEnergie() {

    // Disponibilité de l'accumulateur:
    energie.accumulateurDisponible = 0;
    if ( (etatAccumulateur == UTILISABLE) || (etatAccumulateur == UTILISABLE_MAIS_FAIBLE)) {
        energie.accumulateurDisponible = 1;
    }

    // Chargement ou sollicitation de l'accumulateur:
    energie.chargerAccumulateur = 0;
    energie.solliciterAccumulateur = 0;
    energie.isolerAccumulateur = 0;
    if (etatAlimentation == PRESENTE) {
        if ( (etatAccumulateur == PAS_UTILISABLE) || (etatAccumulateur == UTILISABLE_MAIS_FAIBLE)) {
            energie.chargerAccumulateur = 1;
        }
    } else {
        if ( (etatAccumulateur == UTILISABLE) || (etatAccumulateur == UTILISABLE_MAIS_FAIBLE)) {
            if (etatRaspberry == PROBABLEMENT_ACTIF) {
                energie.solliciterAccumulateur = 1;
            }
        } 
        // Si l'accumulateur n'est pas utilisable, autant l'isoler:
        else {
            energie.isolerAccumulateur = 1;
        }
        
        // Si le raspberry est éteint, on peut isoler l'accumulateur:
        if (etatRaspberry == INACTIF) {
            energie.isolerAccumulateur = 1;
        }
    }

    // Rend l'état de l'accumulateur.
    return &energie;
}

Energie *mesureAlimentation(unsigned char v) {
    switch(etatAlimentation) {
        case PRESENTE:
            // Si l'alimentation tombe en dessous du 7.05V, elle n'est plus
            // utilisable.
            if (v < 180) {
                etatAlimentation = DEFAILLANTE;
            }
            break;

        case DEFAILLANTE:
            // Si l'alimentation remonte au dessus de 7.8V, elle est
            // utilisable à nouveau.
            if (v > 198) {
                etatAlimentation = PRESENTE;
                etatRaspberry = PROBABLEMENT_ACTIF;
            }
            break;
    }

    return etatEnergie();
}

Energie *mesureBoost(unsigned char v) {
    switch(etatRaspberry) {
        // Si la tension de sortie du convertisseur boost dépasse 9.5V, c'est
        // parce que le raspberry a cessé de consommer du courant.
        case PROBABLEMENT_ACTIF:
            if (v > 241) {
                etatRaspberry = INACTIF;
            }
            break;
    }
    return etatEnergie();
}

Energie *mesureAccumulateur(unsigned char vAccumulateur) {
    if ( (vAccumulateur < 50) || (vAccumulateur > 114)) {
        etatAccumulateur = ABSENT;
    } else {
        switch(etatAccumulateur) {
            case ABSENT:
                etatAccumulateur = PAS_UTILISABLE;
                break;
            case PAS_UTILISABLE:
                if (vAccumulateur > 80) {
                    etatAccumulateur = UTILISABLE_MAIS_FAIBLE;
                }
                if (vAccumulateur > 91) {
                    etatAccumulateur = UTILISABLE;
                }
                break;
            case UTILISABLE_MAIS_FAIBLE:
                if (vAccumulateur < 80) {
                    etatAccumulateur = PAS_UTILISABLE;
                }
                if (vAccumulateur > 107) {
                    etatAccumulateur = UTILISABLE;
                }
                break;
            case UTILISABLE:
                if (vAccumulateur < 91) {
                    etatAccumulateur = UTILISABLE_MAIS_FAIBLE;   
                }
                if (vAccumulateur < 80) {
                    etatAccumulateur = PAS_UTILISABLE;
                }
                break;
        }
    }

    return etatEnergie();
}


#ifdef TEST

// Conversion d'une tension x10 à la sortie d'un diviseur
// de tension (1/2) et convertie à 8 bits avec une référence de 5V.
// Exemple: 5 Volts ==> 50 ==> 127
#define CONVERSION_8BITS(x) ((x * 255)/100)

static void peut_completer_un_cycle_de_charge() {
    initialiseEnergie();    
    verifieEgalite("ACCCY01", mesureAccumulateur(CONVERSION_8BITS(42))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY02", mesureAccumulateur(CONVERSION_8BITS(39))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY03", mesureAccumulateur(CONVERSION_8BITS(36))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY04", mesureAccumulateur(CONVERSION_8BITS(35))->chargerAccumulateur, 1);
    verifieEgalite("ACCCY05", mesureAccumulateur(CONVERSION_8BITS(39))->chargerAccumulateur, 1);
    verifieEgalite("ACCCY06", mesureAccumulateur(CONVERSION_8BITS(42))->chargerAccumulateur, 1);
    verifieEgalite("ACCCY07", mesureAccumulateur(CONVERSION_8BITS(43))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY08", mesureAccumulateur(CONVERSION_8BITS(42))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY09", mesureAccumulateur(CONVERSION_8BITS(38))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY10", mesureAccumulateur(CONVERSION_8BITS(36))->chargerAccumulateur, 0);
    verifieEgalite("ACCCY11", mesureAccumulateur(CONVERSION_8BITS(35))->chargerAccumulateur, 1);    
}

static void ne_recommence_pas_un_cycle_de_charge_si_le_precedent_est_interrompu() {
    initialiseEnergie();    
    verifieEgalite("ACCCI01", mesureAccumulateur(CONVERSION_8BITS(42))->chargerAccumulateur, 0);
    verifieEgalite("ACCCI02", mesureAccumulateur(CONVERSION_8BITS(35))->chargerAccumulateur, 1);
    verifieEgalite("ACCCI03", mesureAccumulateur(CONVERSION_8BITS(39))->chargerAccumulateur, 1);

    initialiseEnergie();    
    verifieEgalite("ACCCI04", mesureAccumulateur(CONVERSION_8BITS(39))->chargerAccumulateur, 0);
    verifieEgalite("ACCCI05", mesureAccumulateur(CONVERSION_8BITS( 0))->chargerAccumulateur, 0);
    verifieEgalite("ACCCI06", mesureAccumulateur(CONVERSION_8BITS(39))->chargerAccumulateur, 0);
}

static void peut_detecter_que_l_accumulateur_est_disponible() {
    initialiseEnergie();
    
    verifieEgalite("ACCDI01", mesureAccumulateur(CONVERSION_8BITS(31))->accumulateurDisponible, 0);
    verifieEgalite("ACCDI02", mesureAccumulateur(CONVERSION_8BITS(32))->accumulateurDisponible, 1);
    verifieEgalite("ACCDI03", mesureAccumulateur(CONVERSION_8BITS(42))->accumulateurDisponible, 1);
    verifieEgalite("ACCDI04", mesureAccumulateur(CONVERSION_8BITS(43))->accumulateurDisponible, 1);
}

static void sollicite_l_accumulateur_si_l_alimentation_fait_defaut() {
    initialiseEnergie();
    mesureAccumulateur(CONVERSION_8BITS(40));
    verifieEgalite("ACCSOL01", mesureAlimentation(CONVERSION_8BITS(71))->solliciterAccumulateur, 0);
    verifieEgalite("ACCSOL02", mesureAlimentation(CONVERSION_8BITS(70))->solliciterAccumulateur, 1);
    verifieEgalite("ACCSOL03", mesureAlimentation(CONVERSION_8BITS(71))->solliciterAccumulateur, 1);
    verifieEgalite("ACCSOL04", mesureAlimentation(CONVERSION_8BITS(78))->solliciterAccumulateur, 1);
    verifieEgalite("ACCSOL04", mesureAlimentation(CONVERSION_8BITS(79))->solliciterAccumulateur, 0);
    verifieEgalite("ACCSOL05", mesureAlimentation(CONVERSION_8BITS(78))->solliciterAccumulateur, 0);
}

static void isole_l_accumulateur_si_le_raspberry_s_eteint() {
    initialiseEnergie();
    mesureAccumulateur(CONVERSION_8BITS(40));
    mesureAlimentation(CONVERSION_8BITS(60));
    verifieEgalite("ACCIS01", mesureBoost(CONVERSION_8BITS(85))->isolerAccumulateur, 0);
    verifieEgalite("ACCIS02", mesureBoost(CONVERSION_8BITS(90))->isolerAccumulateur, 0);
    verifieEgalite("ACCIS03", mesureBoost(CONVERSION_8BITS(95))->isolerAccumulateur, 1);
}

static void isole_l_accumulateur_si_pas_disponible_quand_l_alimentation_fait_defaut() {
    initialiseEnergie();
    mesureAlimentation(CONVERSION_8BITS(60));

    verifieEgalite("ACCIA01", mesureAccumulateur(CONVERSION_8BITS(40))->isolerAccumulateur, 0);
    verifieEgalite("ACCIA02", mesureAccumulateur(CONVERSION_8BITS(28))->isolerAccumulateur, 1);
}

static void ne_sollicite_plus_l_accumulateur_si_le_raspberry_s_eteint() {
    initialiseEnergie();
    mesureAccumulateur(CONVERSION_8BITS(40));
    mesureAlimentation(CONVERSION_8BITS(60));
    verifieEgalite("ACCAU01", mesureBoost(CONVERSION_8BITS(85))->solliciterAccumulateur, 1);
    verifieEgalite("ACCAU02", mesureBoost(CONVERSION_8BITS(90))->solliciterAccumulateur, 1);
    verifieEgalite("ACCAU03", mesureBoost(CONVERSION_8BITS(95))->solliciterAccumulateur, 0);
}

static void assume_que_le_raspberry_s_allume_si_l_alimentation_revient() {
    initialiseEnergie();

    mesureAccumulateur(CONVERSION_8BITS(40));   // L'accumulateur est prêt.
    mesureAlimentation(CONVERSION_8BITS(60));   // L'alimentation défaille.
    mesureBoost(CONVERSION_8BITS(95));          // Le convertisseur sature car pas de raspberry.
    
    mesureAlimentation(CONVERSION_8BITS(80));   // L'alimentation est de retour.
    mesureAlimentation(CONVERSION_8BITS(60));   // L'alimentation est repartie.
    
    // On sollicite quand même l'accumulateur:
    verifieEgalite("ACCREV01", mesureBoost(CONVERSION_8BITS(60))->solliciterAccumulateur, 1);
    
    // Si le convertisseur sature encore, on l'arrête:
    verifieEgalite("ACCREV02", mesureBoost(CONVERSION_8BITS(95))->solliciterAccumulateur, 0);
}

static void ne_solicite_plus_l_accumulateur_si_il_est_pas_disponible() {
    initialiseEnergie();
    
    mesureAccumulateur(CONVERSION_8BITS(41));
    verifieEgalite("ACCSL01", mesureAlimentation(CONVERSION_8BITS(59))->solliciterAccumulateur, 1);
    
    mesureAccumulateur(CONVERSION_8BITS(32));
    verifieEgalite("ACCSL02", mesureAlimentation(CONVERSION_8BITS(59))->solliciterAccumulateur, 1);

    mesureAccumulateur(CONVERSION_8BITS(31));
    verifieEgalite("ACCSL03", mesureAlimentation(CONVERSION_8BITS(59))->solliciterAccumulateur, 0);

    mesureAccumulateur(CONVERSION_8BITS(32));
    verifieEgalite("ACCSL04", mesureAlimentation(CONVERSION_8BITS(59))->solliciterAccumulateur, 1);
}

static void ne_solicite_pas_l_accumulateur_si_il_est_pas_disponible() {
    initialiseEnergie();

    mesureAccumulateur(CONVERSION_8BITS(31));
    verifieEgalite("ACCSD01", mesureAlimentation(CONVERSION_8BITS(59))->solliciterAccumulateur, 0);
    
}

void testeEnergie() {
    peut_detecter_que_l_accumulateur_est_disponible();
    peut_completer_un_cycle_de_charge();
    ne_recommence_pas_un_cycle_de_charge_si_le_precedent_est_interrompu();
    
    sollicite_l_accumulateur_si_l_alimentation_fait_defaut();
    ne_sollicite_plus_l_accumulateur_si_le_raspberry_s_eteint();
    assume_que_le_raspberry_s_allume_si_l_alimentation_revient();

    isole_l_accumulateur_si_le_raspberry_s_eteint();
    isole_l_accumulateur_si_pas_disponible_quand_l_alimentation_fait_defaut();

    ne_solicite_plus_l_accumulateur_si_il_est_pas_disponible();
    ne_solicite_pas_l_accumulateur_si_il_est_pas_disponible();    
}

#endif