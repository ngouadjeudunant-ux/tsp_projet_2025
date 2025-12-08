/* algo_rw.c
 * Implémente la marche aléatoire (Random Walk) : génère des tournées au hasard.
 * Paramètres : nombre d'itérations, seed ; garde la meilleure trouvée.
 * Entrée : Instance ; Sortie : meilleure Tour trouvée après N tirages.
 */

#include <stdlib.h>
#include <time.h>
#include "algo_rw.h"
#include "tsp_parser.h"

 #include <stdlib.h>
#include <time.h>
#include "algo_rw.h"

int* rw_tour(const TSP_Instance *inst) {
    srand(time(NULL));

    int n = inst->dimension;
    if (n <= 0 || !inst->dist) return NULL;

    int *tour = malloc((n + 1) * sizeof(int));  // alloue une case de plus
    int *visite = malloc(n * sizeof(int));
    if (!tour || !visite) return NULL;

    for (int i = 0; i < n; i++)
        visite[i] = i;

    int remaining = n;

    for (int i = 0; i < n; i++) {
        int pick = rand() % remaining;
        tour[i] = visite[pick];

        for (int j = pick; j < remaining - 1; j++)
            visite[j] = visite[j + 1];

        remaining--;
    }

    tour[n] = tour[0];  // on boucle le tour (retour à la ville de départ)
    free(visite);
    return tour;
}
