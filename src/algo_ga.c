/* algo_ga.c
 * Implémente un algorithme génétique pour le TSP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "algo_ga.h"
#include "tsp_parser.h"

/* ---------- Type interne pour un individu ---------- */

typedef struct {
    int    n;      /* nombre de villes */
    int   *perm;   /* permutation de 0..n-1 (sans retour explicite) */
    double fitness;/* longueur de la tournée */
} GA_Individual;

/* ---------- Outils aléatoires ---------- */

static int rand_int(int a, int b) {
    /* entier uniforme entre a et b inclus */
    return a + rand() % (b - a + 1);
}

/* ---------- Longueur d'une permutation (tour TSP) ---------- */

static double ga_tour_length(const TSP_Instance *inst, const int *perm) {
    int n = inst->dimension;
    double total = 0.0;

    for (int i = 0; i < n - 1; ++i) {
        int u = perm[i];
        int v = perm[i + 1];
        total += inst->dist[u * n + v];
    }
    /* retour à la ville de départ */
    total += inst->dist[perm[n - 1] * n + perm[0]];

    return total;
}

/* ---------- Génération de permutations aléatoires ---------- */

static void generate_random_perm(int *perm, int n) {
    /* initialiser perm = 0..n-1 */
    for (int i = 0; i < n; ++i) {
        perm[i] = i;
    }
    /* Fisher-Yates */
    for (int i = n - 1; i > 0; --i) {
        int j = rand_int(0, i);
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
}

/* ---------- Mutation par swap ---------- */

static void swap_mutation(int *perm, int n, double mutation_rate) {
    for (int i = 0; i < n; ++i) {
        double r = (double)rand() / (double)RAND_MAX;
        if (r < mutation_rate) {
            int j = rand_int(0, n - 1);
            int tmp = perm[i];
            perm[i] = perm[j];
            perm[j] = tmp;
        }
    }
}

/* ---------- Croisement OX (Ordered Crossover) ---------- */

static void ordered_crossover(const int *parent1,
                              const int *parent2,
                              int *child,
                              int n)
{
    /* Initialiser l'enfant à -1 (valeur impossible) */
    for (int i = 0; i < n; ++i) {
        child[i] = -1;
    }

    int start = rand_int(0, n - 1);
    int end   = rand_int(0, n - 1);
    if (start > end) {
        int tmp = start;
        start   = end;
        end     = tmp;
    }

    /* Copier le segment de parent1 */
    for (int i = start; i <= end; ++i) {
        child[i] = parent1[i];
    }

    /* Compléter avec parent2 dans l'ordre, en évitant les doublons */
    int idx_child = (end + 1) % n;

    for (int k = 0; k < n; ++k) {
        int idx_p2 = (end + 1 + k) % n;
        int gene   = parent2[idx_p2];

        /* vérifier si gene est déjà dans child */
        int used = 0;
        for (int j = 0; j < n; ++j) {
            if (child[j] == gene) {
                used = 1;
                break;
            }
        }
        if (!used) {
            child[idx_child] = gene;
            idx_child = (idx_child + 1) % n;
        }
    }
}

/* ---------- Sélection par tournoi ---------- */

static int tournament_select_index(GA_Individual *pop,
                                   int pop_size,
                                   int tournament_size)
{
    int best_idx = -1;
    for (int k = 0; k < tournament_size; ++k) {
        int idx = rand_int(0, pop_size - 1);
        if (best_idx == -1 || pop[idx].fitness < pop[best_idx].fitness) {
            best_idx = idx;
        }
    }
    return best_idx;
}

/* ---------- Copie d'un individu ---------- */

static void copy_individual(const GA_Individual *src,
                            GA_Individual *dst)
{
    dst->n       = src->n;
    dst->fitness = src->fitness;
    memcpy(dst->perm, src->perm, src->n * sizeof(int));
}

/* ============================================================
 *  Fonction principale : ga_tour
 * ============================================================ */

int* ga_tour(const TSP_Instance *inst, int pop_size, int generations, double mutation_rate)
{
    if (!inst || inst->dimension <= 0 || !inst->dist) {
        return NULL;
    }

    int n = inst->dimension;

    if (pop_size < 2) pop_size = 2;
    if (generations < 1) generations = 1;
    if (mutation_rate < 0.0) mutation_rate = 0.0;
    if (mutation_rate > 1.0) mutation_rate = 1.0;

    int tournament_size = pop_size / 2;
    if (tournament_size < 1) tournament_size = 1;

    /* Seed du RNG (comme rw_tour le fait de son côté) */
    srand((unsigned int)time(NULL));

    /* Allocation populations */
    GA_Individual *population = malloc(pop_size * sizeof(GA_Individual));
    GA_Individual *offspring  = malloc(pop_size * sizeof(GA_Individual));
    if (!population || !offspring) {
        fprintf(stderr, "Erreur malloc population GA\n");
        free(population);
        free(offspring);
        return NULL;
    }

    for (int i = 0; i < pop_size; ++i) {
        population[i].n = n;
        population[i].perm = malloc(n * sizeof(int));
        offspring[i].n = n;
        offspring[i].perm = malloc(n * sizeof(int));

        if (!population[i].perm || !offspring[i].perm) {
            fprintf(stderr, "Erreur malloc perm GA\n");
            for (int k = 0; k <= i; ++k) {
                free(population[k].perm);
                free(offspring[k].perm);
            }
            free(population);
            free(offspring);
            return NULL;
        }
    }

    /* Initialisation aléatoire de la population */
    for (int i = 0; i < pop_size; ++i) {
        generate_random_perm(population[i].perm, n);
        population[i].fitness = ga_tour_length(inst, population[i].perm);
    }

    /* Meilleur global */
    GA_Individual best_global;
    best_global.n = n;
    best_global.perm = malloc(n * sizeof(int));
    if (!best_global.perm) {
        fprintf(stderr, "Erreur malloc best_global GA\n");
        for (int i = 0; i < pop_size; ++i) {
            free(population[i].perm);
            free(offspring[i].perm);
        }
        free(population);
        free(offspring);
        return NULL;
    }

    int best_idx = 0;
    for (int i = 1; i < pop_size; ++i) {
        if (population[i].fitness < population[best_idx].fitness) {
            best_idx = i;
        }
    }
    copy_individual(&population[best_idx], &best_global);

    /* Boucle des générations (variante "simple" avec élitisme) */
    for (int gen = 0; gen < generations; ++gen) {

        /* Création des enfants via sélection + crossover + mutation */
        for (int i = 0; i < pop_size; ++i) {
            int p1_idx = tournament_select_index(population, pop_size, tournament_size);
            int p2_idx = tournament_select_index(population, pop_size, tournament_size);

            GA_Individual *p1 = &population[p1_idx];
            GA_Individual *p2 = &population[p2_idx];
            GA_Individual *c  = &offspring[i];

            ordered_crossover(p1->perm, p2->perm, c->perm, n);
            swap_mutation(c->perm, n, mutation_rate);
            c->fitness = ga_tour_length(inst, c->perm);
        }

        /* Mise à jour du meilleur global */
        int best_gen_idx = 0;
        for (int i = 1; i < pop_size; ++i) {
            if (offspring[i].fitness < offspring[best_gen_idx].fitness) {
                best_gen_idx = i;
            }
        }
        if (offspring[best_gen_idx].fitness < best_global.fitness) {
            copy_individual(&offspring[best_gen_idx], &best_global);
        }

        /* Élitisime : imposer best_global dans la population enfant
           (remplace le pire individu) */
        int worst_idx = 0;
        for (int i = 1; i < pop_size; ++i) {
            if (offspring[i].fitness > offspring[worst_idx].fitness) {
                worst_idx = i;
            }
        }
        copy_individual(&best_global, &offspring[worst_idx]);

        /* On échange population <-> offspring */
        GA_Individual *tmp = population;
        population = offspring;
        offspring  = tmp;
    }

    /* Construire la tournée finale au format n+1 (dernier = premier) */
    int *tour = malloc((n + 1) * sizeof(int));
    if (!tour) {
        fprintf(stderr, "Erreur malloc tour GA final\n");
        tour = NULL; /* on continue pour libérer proprement */
    } else {
        /* best_global.perm contient des villes 0..n-1 */
        for (int i = 0; i < n; ++i) {
            tour[i] = best_global.perm[i];
        }
        tour[n] = best_global.perm[0]; /* retour à la ville de départ */
    }

    /* Libération mémoire */
    for (int i = 0; i < pop_size; ++i) {
        free(population[i].perm);
        free(offspring[i].perm);
    }
    free(population);
    free(offspring);
    free(best_global.perm);

    return tour;
}
