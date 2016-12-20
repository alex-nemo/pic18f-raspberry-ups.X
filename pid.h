#ifndef PID__H
#define PID__H

typedef struct {
    unsigned char p;
    unsigned char i;
    unsigned char d;
    
    int erreurI;
    int erreurD;
    int erreurP;

    int magnitude;
} PID;

unsigned char calculatePID(PID *pid, unsigned char valeurMesuree, unsigned char valeurDemandee);

#ifdef TEST
void testPid();
#endif


#endif