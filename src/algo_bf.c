/* algo_bruteforce.c
 * Implémente la recherche exhaustive (génération lexicographique des permutations).
 * Met à jour la meilleure tournée trouvée ; doit respecter le handler Ctrl-C.
 * Entrée : Instance + dist_fct ; Sortie : meilleure Tour et sa longueur.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include "algo_bf.h"

// variable globale définie dans main.c
extern volatile sig_atomic_t stop_requested;

static void permute(int nb_nodes, int pos, int *perm, bool *used,
                    int *best_perm, unsigned long long *count_best,
                    void *(*cout)(void *, int *)) {

    // Si Ctrl-C → on arrête immédiatement la récursion
    if (stop_requested)
        return;

    if (pos == nb_nodes) {
        void *res = cout(NULL, perm);
        if (res != NULL) {
            unsigned long long cost = *(unsigned long long *)res;
            if (cost < *count_best) {
                *count_best = cost;
                memcpy(best_perm, perm, nb_nodes * sizeof(int));
            }
            free(res);
        }
        return;
    }

    for (int i = 0; i < nb_nodes; ++i) {
        if (stop_requested)
            return;

        if (!used[i]) {
            perm[pos] = i;
            used[i] = true;

            permute(nb_nodes, pos + 1, perm, used, best_perm, count_best, cout);

            used[i] = false;

            if (stop_requested)
                return;
        }
    }
}

double brute(int nb_nodes,
             int nb_ressources,
             int *best_perm,
             unsigned long long *count_best,
             void *(*cout)(void *, int *)) {

    (void)nb_ressources;

    int *perm = malloc(nb_nodes * sizeof(int));
    bool *used = calloc(nb_nodes, sizeof(bool));
    *count_best = ~0ULL;

    permute(nb_nodes, 0, perm, used, best_perm, count_best, cout);

    free(perm);
    free(used);

    return (double)(*count_best);
}
