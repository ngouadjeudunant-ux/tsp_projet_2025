/* algo_nn.c
 * Implémente l'algorithme "Plus Proche Voisin" (NN) pour construire une tournée.
 * Politique : choisir un sommet de départ, sélectionner le plus proche non-visité.
 * Entrée : Instance + dist_fct ; Sortie : tournée construite (Tour).
 */
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "algo_nn.h"

int* nn_tour(const TSP_Instance *inst) {
    int n = inst->dimension;
    if (n <= 0 || !inst->dist) return NULL;

    int *tour = malloc((n + 1) * sizeof(int));
    int *visite = calloc(n, sizeof(int));
    if (!tour || !visite) return NULL;

    int courant = 0;
    tour[0] = 0;
    visite[0] = 1;

    for (int k = 1; k < n; ++k) {
        double min = DBL_MAX;
        int prochain = -1;
        for (int j = 0; j < n; ++j) {
            if (!visite[j] && inst->dist[courant * n + j] < min) {
                min = inst->dist[courant * n + j];
                prochain = j;
            }
        }
        tour[k] = prochain;
        visite[prochain] = 1;
        courant = prochain;
    }

    tour[n] = 0;  // retour au départ

    free(visite);
    return tour;
}

double tour_length(const TSP_Instance *inst, int *tour) {
    double total = 0;
    int n = inst->dimension;
    for (int i = 0; i < n; ++i)
        total += inst->dist[tour[i] * n + tour[i + 1]];
    return total;
}
