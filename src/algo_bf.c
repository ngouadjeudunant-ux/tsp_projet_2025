/* algo_bruteforce.c
 * Implémente la recherche exhaustive (génération lexicographique des permutations).
 * Met à jour la meilleure tournée trouvée ; doit respecter le handler Ctrl-C.
 * Entrée : Instance + dist_fct ; Sortie : meilleure Tour et sa longueur.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>     // pour DBL_MAX
#include "algo_bf.h"

/**
 * @brief Fonction récursive qui explore toutes les permutations de la tournée.
 * @param inst      Pointeur vers l'instance TSP (contient inst->n et inst->dist).
 * @param current   Tableau courant de la tournée partielle.
 * @param visited   Tableau booléen des villes déjà visitées.
 * @param pos       Position courante dans la tournée à remplir.
 * @param best_tour Tableau de sortie pour la meilleure tournée trouvée.
 * @param best_cost Pointeur vers le coût minimal trouvé.
 */
static void explore(const TSP_Instance *inst, int *current, bool *visited, int pos,
                    int *best_tour, double *best_cost) {
    int n = inst->dimension;

    if (pos == n) {
        // Calculer le coût de la tournée (incluant retour à la ville de départ)
        double cost = 0.0;
        for (int i = 0; i < n - 1; ++i) {
            cost += inst->dist[current[i] * n + current[i + 1]];
        }
        cost += inst->dist[current[n - 1] * n + current[0]]; // retour à 0

        if (cost < *best_cost) {
            *best_cost = cost;
            for (int i = 0; i < n; ++i)
                best_tour[i] = current[i];
        }
        return;
    }

    for (int city = 1; city < n; ++city) {
        if (!visited[city]) {
            current[pos] = city;
            visited[city] = true;
            explore(inst, current, visited, pos + 1, best_tour, best_cost);
            visited[city] = false;  // backtracking
        }
    }
}

/**
 * Lance la résolution brute-force : toutes les permutations sont explorées.
 */
void bf_solve(const TSP_Instance *inst, int *best_tour, double *best_cost) {
    int n = inst->dimension;

    if (n <= 1) {
        if (n == 1) {
            best_tour[0] = 0;
            *best_cost = 0.0;
        }
        return;
    }

    int *current = malloc(n * sizeof(int));
    bool *visited = calloc(n, sizeof(bool));

    current[0] = 0;
    visited[0] = true;
    *best_cost = DBL_MAX;

    explore(inst, current, visited, 1, best_tour, best_cost);

    free(current);
    free(visited);
}
 