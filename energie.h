#ifndef ACCUMULATEUR_H
#define	ACCUMULATEUR_H

/**
 * Indique l'état de l'administration d'énergie.
 */
typedef struct {
    /** Indique que l'accumulateur est physiquement présent sur le circuit. */
    unsigned char accumulateurPresent : 1;
    /** Indique qu'il faut charger l'accumulateur.*/
    unsigned char chargerAccumulateur : 1;
    /** Indique que l'accumulateur a de la charge disponible. */
    unsigned char accumulateurDisponible : 1;
    /** Indique qu'il faut puiser l'energie de l'accumulateur. */
    unsigned char solliciterAccumulateur  : 1;
} Energie;

/**
 * Initialise l'état de l'administration d'énergie.
 */
void initialiseEnergie();

/**
 * Administre l'énergie à partir de la tension d'alimentation. 
 * @param vacc Tension d'alimentation, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return 
 */
Energie *mesureAlimentation(unsigned char vacc);

/**
 * Administre l'énergie à partir de sa tension de sortie.
 * @param vacc Tension de sortie de l'accumulateur, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return État actuel de l'accumulateur. La fonction appelante est responsable
 * de le propager sur le circuit.
 */
Energie *mesureAccumulateur(unsigned char vacc);


/**
 * Administre l'énergie à partir de la tension de sortie du convertisseur Boost.
 * @param vacc Tension de sortie du convertisseur Boost, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return État actuel de l'accumulateur. La fonction appelante est responsable
 * de le propager sur le circuit.
 */
Energie *mesureBoost(unsigned char vboost);

#ifdef TEST
void testeEnergie();
#endif

#endif

