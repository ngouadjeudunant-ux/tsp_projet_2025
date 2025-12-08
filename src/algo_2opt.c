/* algo_2opt.c
 * Optimisation locale 2-opt pour améliorer une tournée du TSP.
 * Entrée : instance + une tournée valide.
 * Sortie : 1 si amélioration, 0 sinon. Le tour est modifié en place.
 */

#include <stdio.h>
#include <stdlib.h>
#include "algo_2opt.h"

static inline double dist(const TSP_Instance *inst, int i, int j) {
    int n = inst->dimension;
    return inst->dist[i * n + j];
}

/**
 * Inverse le segment tour[i..j] (opération clé du 2-opt)
 */
static void reverse_segment(int *tour, int i, int j) {
    while (i < j) {
        int tmp = tour[i];
        tour[i] = tour[j];
        tour[j] = tmp;
        i++;
        j--;
    }
}

/**
 * Amélioration 2-opt :
 * On teste toutes les paires (i, j) et on applique l'inversion si le coût diminue.
 */
int improve_2opt(const TSP_Instance *inst, int *tour) {
    int n = inst->dimension;
    int improved = 0;

    while (1) {
        double best_gain = 0.0;
        int best_i = -1, best_j = -1;

        for (int i = 0; i < n - 2; i++) {
            int A = tour[i];
            int B = tour[i + 1];

            for (int j = i + 2; j < n; j++) {
                int C = tour[j];
                int D = (j + 1 == n) ? tour[0] : tour[j + 1];

                // Coûts avant et après inversion
                double old_cost = dist(inst, A, B) + dist(inst, C, D);
                double new_cost = dist(inst, A, C) + dist(inst, B, D);
                double gain = old_cost - new_cost;

                if (gain > best_gain) {
                    best_gain = gain;
                    best_i = i;
                    best_j = j;
                }
            }
        }

        if (best_gain > 0) {
            reverse_segment(tour, best_i + 1, best_j);
            improved = 1;
        } else {
            break; // stable
        }
    }

    return improved;
}
