/* main.c
 * Point d'entrée : parse les options CLI (-f, -m, -o, -c, -h) et orchestre l'exécution.
 * Appelle : parse_tsp(), select_dist(), construction de la tournée, algorithmes, affichage.
 * Entrées : arguments argv/argc ; Sorties : affichage CSV/console et code de sortie.
 * Compilation : gcc src/*.c -Iinclude -o tsp.exe -lm
 * 
 * executer : ./tsp.exe -f tests/data/att10.tsp -m nn   (avec plus proche voisin)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsp_parser.h"
#include "algo_nn.h"

void usage(const char *prog) {
    printf("Usage : %s -f <fichier.tsp> -m nn\n", prog);
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

    // Sélection de l’algorithme
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
    } else {
        fprintf(stderr, "Méthode '%s' non supportée.\n", methode);
        usage(argv[0]);
    }

    tsp_free_instance(inst);
    return 0;
}
