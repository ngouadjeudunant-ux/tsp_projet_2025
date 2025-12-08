/*
 * Test de la fonction brute-force générique.
 * Ce module résout un problème d’affectation simple :
 * attribuer des tâches à des personnes de façon à minimiser le coût total.
 * Il utilise la fonction brute() pour explorer toutes les permutations possibles,
 * affiche chaque permutation testée avec son coût, puis affiche la meilleure solution.
 * Ce test démontre la généricité de l’algorithme brute-force, sans lien avec le TSP.
 */
// Compilation :  gcc tests/bf_test.c src/algo_bf.c -Iinclude -o tests/bf_test
// execution :  ./tests/bf_test

#include <stdio.h>
#include <stdlib.h>
#include "algo_bf.h"  //  le bon header pour la fonction générique
#include <signal.h>

#define N 3
 
volatile sig_atomic_t stop_requested = 0;

// Définition de la matrice des coûts [personne][tâche]
int cost_matrix[N][N] = {
    {9, 2, 7},
    {6, 4, 3},
    {5, 8, 1}
};

// Fonction de coût pour brute()
void *assignment_cost(void *unused, int *perm) {
    (void)unused;
    unsigned long long *cost = malloc(sizeof(unsigned long long));
    *cost = 0;

    for (int i = 0; i < N; ++i) {
        *cost += cost_matrix[i][perm[i]];  // personne i → tâche perm[i]
    }

    // Affichage de la permutation testée
    printf("Permutation testee : ");
    for (int i = 0; i < N; ++i)
        printf("%d ", perm[i]);
    printf("-> Cout : %llu\n", *cost);

    return cost;
}

int main() {
    int best_perm[N];
    unsigned long long best_cost;

    brute(N, 0, best_perm, &best_cost, assignment_cost);

    printf("\n Meilleure affectation (personne -> tache) :\n");
    for (int i = 0; i < N; ++i)
        printf("  Personne %d -> Tache %d\n", i, best_perm[i]);
    printf("Cout total : %llu\n", best_cost);

    return 0;
}
