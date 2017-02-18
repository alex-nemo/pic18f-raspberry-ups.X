#ifndef ACCUMULATEUR_H
#define	ACCUMULATEUR_H

/**
 * Indique l'état de l'accumulateur.
 */
typedef struct {
    /** Indique que l'accumulateur est physiquement présent sur le circuit. */
    unsigned char accumulateurPresent : 1;
    /** Indique que l'accumulateur doit être mis en charge.*/
    unsigned char accumulateurEnCharge : 1;
    /** Indique que l'accumulateur a de la charge disponible. */
    unsigned char accumulateurDisponible : 1;
    /** Indique que l'accumulateur est sollicité. */
    unsigned char accumulateurSollicite  : 1;
} Accumulateur;

/**
 * Initialise l'état de l'accumulateur.
 */
void initialiseAccumulateur();

/**
 * Gère l'accumulateur à partir de la tension d'alimentation. 
 * @param vacc Tension d'alimentation, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return 
 */
Accumulateur *mesureAlimentation(unsigned char vacc);

/**
 * Gère l'accumulateur à partir de sa tension de sortie.
 * @param vacc Tension de sortie de l'accumulateur, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return État actuel de l'accumulateur. La fonction appelante est responsable
 * de le propager sur le circuit.
 */
Accumulateur *mesureAccumulateur(unsigned char vacc);


/**
 * Gère l'accumulateur à partir de la tension de sortie du convertisseur Boost.
 * @param vacc Tension de sortie du convertisseur Boost, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return État actuel de l'accumulateur. La fonction appelante est responsable
 * de le propager sur le circuit.
 */
Accumulateur *mesureBoost(unsigned char vboost);

#ifdef TEST
void testeAccumulateur();
#endif

#endif

