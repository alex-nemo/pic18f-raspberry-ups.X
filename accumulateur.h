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
} Accumulateur;

/**
 * Initialise l'état de l'accumulateur.
 */
void initialiseAccumulateur();

/**
 * Gère l'accumulateur.
 * @param vacc Tension de sortie de l'accumulateur, obtenue au travers d'un 
 * diviseur de tension 1/2, puis numérisée sur 8 bits.
 * @return État actuel de l'accumulateur. La fonction appelante est responsable
 * de le propager sur le circuit.
 */
Accumulateur *machineAccumulateur(unsigned char vacc);

#ifdef TEST
void testeAccumulateur();
#endif

#endif

