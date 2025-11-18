/* algo_rw.c
 * Implémente la marche aléatoire (Random Walk) : génère des tournées au hasard.
 * Paramètres : nombre d'itérations, seed ; garde la meilleure trouvée.
 * Entrée : Instance ; Sortie : meilleure Tour trouvée après N tirages.
 */

#include <stdlib.h>
#include <time.h>
#include "algo_rw.h"
#include "tsp_parser.h"

void int_array_pop(int ** array, int index, int length){
    for (int i = index; i < length-1; i++){
        (*array)[i] = (*array)[i+1];
    }
}

int* rw_tour(const TSP_Instance *inst) {
    srand(time(NULL));
    int n = inst->dimension;
    int pick;
    if (n <= 0 || !inst->dist) return NULL;
    
    int *tour = malloc(n * sizeof(int));
    int *visite = calloc(n, sizeof(int));
    if (!tour || !visite) return NULL;
    for (int i = 0; i < n; i++) visite[i] = i;

    for (int i = 0; i < n; i++){
        pick = (rand()%(n-i));
        tour[i] = visite[pick];
        int_array_pop(&visite, pick, n-i);
    }
    free(visite);
    return tour;
}