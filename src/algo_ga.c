/* algo_ga.c
 * Implémente un algorithme génétique pour le TSP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "algo_ga.h"
#include "tsp_parser.h"

/* Variable globale provenant de main.c */
extern volatile sig_atomic_t stop_requested;

/*Type interne pour un individu  */

typedef struct {
    int    n;      /* nombre de villes */
    int   *perm;   /* permutation de 0..n-1 (sans retour explicite) */
    double fitness;/* longueur de la tournée */
} GA_Individual;

/* Outils aléatoires */

static int rand_int(int a, int b) {
    return a + rand() % (b - a + 1);
}

/* Longueur d'une permutation (tour TSP) */

static double ga_tour_length(const TSP_Instance *inst, const int *perm) {
    int n = inst->dimension;
    double total = 0.0;

    for (int i = 0; i < n - 1; ++i) {
        int u = perm[i];
        int v = perm[i + 1];
        total += inst->dist[u * n + v];
    }

    total += inst->dist[perm[n - 1] * n + perm[0]];

    return total;
}

/* Génération permutation aléatoire */

static void generate_random_perm(int *perm, int n) {
    for (int i = 0; i < n; ++i)
        perm[i] = i;

    for (int i = n - 1; i > 0; --i) {
        int j = rand_int(0, i);
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
}

/* Mutation swap  */

static void swap_mutation(int *perm, int n, double mutation_rate) {
    for (int i = 0; i < n; ++i) {
        if ((double)rand() / RAND_MAX < mutation_rate) {
            int j = rand_int(0, n - 1);
            int tmp = perm[i];
            perm[i] = perm[j];
            perm[j] = tmp;
        }
    }
}

/* Ordered Crossover (OX) */

static void ordered_crossover(const int *p1, const int *p2, int *child, int n) {

    for (int i = 0; i < n; ++i)
        child[i] = -1;

    int start = rand_int(0, n - 1);
    int end   = rand_int(0, n - 1);
    if (start > end) {
        int tmp = start; start = end; end = tmp;
    }

    for (int i = start; i <= end; ++i)
        child[i] = p1[i];

    int idx = (end + 1) % n;

    for (int k = 0; k < n; ++k) {
        int candidate = p2[(end + 1 + k) % n];
        int used = 0;

        for (int j = 0; j < n; ++j)
            if (child[j] == candidate)
                used = 1;

        if (!used) {
            child[idx] = candidate;
            idx = (idx + 1) % n;
        }
    }
}

/*Sélection par tournoi */

static int tournament_select_index(GA_Individual *pop, int pop_size, int tsize) {
    int best = -1;
    for (int k = 0; k < tsize; ++k) {
        int idx = rand_int(0, pop_size - 1);
        if (best == -1 || pop[idx].fitness < pop[best].fitness)
            best = idx;
    }
    return best;
}

/*Copier un individu */

static void copy_individual(const GA_Individual *src, GA_Individual *dst) {
    dst->n       = src->n;
    dst->fitness = src->fitness;
    memcpy(dst->perm, src->perm, src->n * sizeof(int));
}

/* 
 * ALGORTIHME GÉNÉTIQUE COMPLET
 */

int* ga_tour(const TSP_Instance *inst, int pop_size, int generations, double mutation_rate)
{
    if (!inst || inst->dimension <= 0 || !inst->dist)
        return NULL;

    int n = inst->dimension;

    if (pop_size < 2) pop_size = 2;
    if (generations < 1) generations = 1;

    int tsize = pop_size / 2;
    if (tsize < 1) tsize = 1;

    srand((unsigned int)time(NULL));

    /* Allocation population */
    GA_Individual *pop = malloc(pop_size * sizeof(GA_Individual));
    GA_Individual *childpop = malloc(pop_size * sizeof(GA_Individual));

    if (!pop || !childpop) return NULL;

    for (int i = 0; i < pop_size; ++i) {
        pop[i].n = n;
        childpop[i].n = n;

        pop[i].perm = malloc(n * sizeof(int));
        childpop[i].perm = malloc(n * sizeof(int));

        if (!pop[i].perm || !childpop[i].perm)
            return NULL;
    }

    /* Population initiale */
    for (int i = 0; i < pop_size; ++i) {
        generate_random_perm(pop[i].perm, n);
        pop[i].fitness = ga_tour_length(inst, pop[i].perm);
    }

    /* Meilleur global */
    GA_Individual best;
    best.n = n;
    best.perm = malloc(n * sizeof(int));

    int best_idx = 0;
    for (int i = 1; i < pop_size; ++i)
        if (pop[i].fitness < pop[best_idx].fitness)
            best_idx = i;

    copy_individual(&pop[best_idx], &best);

    /*
     *BOUCLE DES GÉNÉRATIONS
     */
    for (int gen = 0; gen < generations; ++gen) {

        if (stop_requested) break;

        /* Création des enfants */
        for (int i = 0; i < pop_size; ++i) {

            if (stop_requested) break;

            int p1 = tournament_select_index(pop, pop_size, tsize);
            int p2 = tournament_select_index(pop, pop_size, tsize);

            ordered_crossover(pop[p1].perm, pop[p2].perm, childpop[i].perm, n);
            swap_mutation(childpop[i].perm, n, mutation_rate);
            childpop[i].fitness = ga_tour_length(inst, childpop[i].perm);
        }

        if (stop_requested) break;

        /* Trouver meilleur enfant */
        int best_child = 0;
        for (int i = 1; i < pop_size; ++i)
            if (childpop[i].fitness < childpop[best_child].fitness)
                best_child = i;

        /* Mise à jour du meilleur global */
        if (childpop[best_child].fitness < best.fitness)
            copy_individual(&childpop[best_child], &best);

        /* remplace le pire individu */
        int worst = 0;
        for (int i = 1; i < pop_size; ++i)
            if (childpop[i].fitness > childpop[worst].fitness)
                worst = i;

        copy_individual(&best, &childpop[worst]);

        /* swap pop / childpop */
        GA_Individual *tmp = pop;
        pop = childpop;
        childpop = tmp;
    }

    /* 
     * Construire la tournée finale retournée
     */
    int *tour = malloc((n + 1) * sizeof(int));
    for (int i = 0; i < n; ++i)
        tour[i] = best.perm[i];
    tour[n] = best.perm[0];

    /* Libération */
    for (int i = 0; i < pop_size; ++i) {
        free(pop[i].perm);
        free(childpop[i].perm);
    }
    free(pop);
    free(childpop);
    free(best.perm);

    return tour;
}
