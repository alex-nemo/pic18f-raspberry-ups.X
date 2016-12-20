#include "pid.h"
#include "test.h"

unsigned char calculatePID(PID *pid, unsigned char valeurMesuree, unsigned char valeurDemandee) {
    int v;
    int correction;
    int erreurP   = valeurDemandee - valeurMesuree;
    pid->erreurI += erreurP;
    pid->erreurD  = erreurP  - pid->erreurP;
    pid->erreurP  = erreurP;

    correction  = erreurP * pid->p;
    correction += pid->erreurI * pid->i;
    correction += pid->erreurD * pid->d;
    
    pid->magnitude += correction;
    
    if (pid->magnitude < 0) {
        pid->magnitude = 0;
        pid->erreurI = 0;
    }
    if (pid->magnitude > 5088) {
        pid->magnitude = 5088;
        pid->erreurI = 0;
    }
    v = pid->magnitude;
    v >>= 5;
    return (unsigned char) v;
}

#ifdef TEST

typedef struct {
    int gain;
    int masse;
    int echelle;
} ModelePhysique;

/**
 * Représente le modèle physique du système à réguler.
 * Le modèle fonctionne selon l'équation suivante équations:
 * - ΔVo = (Vc - Vo/G) / M
 * Les variables:
 * - Vc - Valeur de contrôle.
 * - Vo - Valeur de sortie.
 * Les constantes:
 * - Le gain du modèle: G - La valeur de sortie du modèle en fonction de 
 * la magnitude, après un temps infini 
 *             ΔVo=0 ==> (Vc - Vo/G) = 0 ==> Vo = G*Vc
 * - L'inertie du modèle: M (pour masse) - L'inverse de l'accélération du modèle 
 * en fonction de la valeur de contrôle, lorsque la valeur de sortie est 0:
 *             Vo=0 ==> ΔVo = Vc/M
 * @param magnitude La valeur appliquée au modèle.
 * @param valeurMesuree La valeur actuelle du modèle.
 * @param iterations Le nombre d'itérations à réaliser sur le modèle.
 * @return La nouvelle valeur du modèle.
 */
unsigned char itereModelePhysique(ModelePhysique *modelePhysique, 
                                  unsigned char valeurControle, 
                                  unsigned char valeurSortie, 
                                  unsigned char iterations) {
    unsigned char k;
    int vc, vo;
    int delta;
    
    vc = valeurControle * modelePhysique->echelle;
    vo = valeurSortie * modelePhysique->echelle;
    for (k = 0; k < iterations; k++) {
        delta = (vc - vo/modelePhysique->gain) / modelePhysique->masse;
        vo += delta;
    }
    vo /= modelePhysique->echelle;
    return (unsigned char) vo;
}
/**
 * valeurMesuree = magnitude * 2.6
 * 
 * a = (valeurMesuree * 26 - magnitude * 10) / 10
 */
void testPidConvergence() {
    PID pid1 = {1, 0, 2, 0, 0, 0, 0};
    ModelePhysique modelePhysique = {26, 5, 10};
    unsigned char n;
    unsigned char valeurControle, valeurSortie = 0;
    
    valeurSortie = 0;
    for (n = 0; n < 100; n++) {
        valeurControle = calculatePID(&pid1, valeurSortie, 56);
        valeurSortie = itereModelePhysique(&modelePhysique, valeurControle, valeurSortie, 3);
    }
    
    testeEgaliteEntiers("PID001", valeurSortie, 56);
}

void testPid() {
    testPidConvergence();
}
#endif
