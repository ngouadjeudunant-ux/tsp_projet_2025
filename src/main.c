// Compilation : gcc src/*.c -Iinclude -o tsp.exe -lm
// Execution exemple : ./tsp.exe -f tests/data/att15.tsp -m nn

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tsp_parser.h"
#include "algo_nn.h"
#include "algo_bf.h"

void usage(const char *prog) {
    printf("Usage : %s -f <fichier.tsp> -m <nn|bf>\n", prog);
}

int main(int argc, char **argv) {
    const char *fichier = NULL;
    const char *methode = NULL;

    // Lecture des options CLI
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-f") && i + 1 < argc)
            fichier = argv[++i];
        else if (!strcmp(argv[i], "-m") && i + 1 < argc)
            methode = argv[++i];
        else if (!strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return 0;
        }
    }

    if (!fichier || !methode) {
        usage(argv[0]);
        return 1;
    }

    // Chargement de l'instance TSP
    TSP_Instance *inst = tsp_read_file(fichier);
    if (!inst) {
        fprintf(stderr, "Erreur lors du chargement de %s\n", fichier);
        return 2;
    }

    // Exécution de l’algorithme choisi
    if (!strcmp(methode, "nn")) {
        int *tour = nn_tour(inst);
        if (tour) {
            double len = tour_length(inst, tour);
            printf("Tournée NN : ");
            for (int i = 0; i <= inst->dimension; ++i)
                printf("%d ", tour[i] + 1);  // affichage 1-based
            printf("\nLongueur : %.0f\n", len);
            free(tour);
        } else {
            fprintf(stderr, "Erreur lors du calcul NN.\n");
        }

    } else if (!strcmp(methode, "bf")) {
        int n = inst->dimension;
        int *best_tour = malloc(n * sizeof(int));
        double best_cost = 0.0;

        bf_solve(inst, best_tour, &best_cost);

        printf("Tournée brute-force : ");
        for (int i = 0; i < n; ++i)
            printf("%d ", best_tour[i] + 1);
        printf("%d\n", best_tour[0] + 1);  // retour au point de départ
        printf("Longueur : %.0f\n", best_cost);
        free(best_tour);

    } else {
        fprintf(stderr, "Méthode '%s' non supportée.\n", methode);
        usage(argv[0]);
    }

    tsp_free_instance(inst);
    return 0;
}
