// Compilation : gcc src/*.c -Iinclude -o tsp.exe -lm
// Execution exemple : ./tsp.exe -f tests/data/att15.tsp -m nn

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tsp_parser.h"
#include "algo_nn.h"
#include "algo_bf.h"
#include "csv_export.h"

void usage(const char *prog) {
    printf("Usage : %s -f <fichier.tsp> -m <nn|bf> [-o <export.csv>]\n", prog);
}

int main(int argc, char **argv) {
    const char *fichier = NULL;
    const char *methode = NULL;
    const char *csv_file = NULL;

    // Lecture options CLI
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-f") && i + 1 < argc)
            fichier = argv[++i];
        else if (!strcmp(argv[i], "-m") && i + 1 < argc)
            methode = argv[++i];
        else if (!strcmp(argv[i], "-o") && i + 1 < argc)
            csv_file = argv[++i];
        else if (!strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return 0;
        }
    }

    if (!fichier || !methode) {
        usage(argv[0]);
        return 1;
    }

    // Charger l’instance
    TSP_Instance *inst = tsp_read_file(fichier);
    if (!inst) {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", fichier);
        return 2;
    }

    int *tour = NULL;
    double length = 0.0;
    clock_t start = clock();

    // Exécution de l’algo
    if (!strcmp(methode, "nn")) {
        tour = nn_tour(inst);
        if (tour)
            length = tour_length(inst, tour);
    } else if (!strcmp(methode, "bf")) {
        tour = malloc((inst->dimension + 1) * sizeof(int));
        if (tour)
            bf_solve(inst, tour, &length);
    } else {
        fprintf(stderr, "Méthode inconnue : %s\n", methode);
        usage(argv[0]);
        tsp_free_instance(inst);
        return 3;
    }

    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;

    // Affichage résultat
    if (tour) {
        printf("Méthode : %s\n", methode);
        printf("Tournée : ");
        for (int i = 0; i <= inst->dimension; ++i)
            printf("%d ", tour[i] + 1);
        printf("\nLongueur : %.0f\nDurée : %.3fs\n", length, elapsed);

        // Export CSV
        if (csv_file) {
            if (export_summary_csv(csv_file, inst->name, methode, elapsed, length, tour, inst->dimension) == 0)
                printf("Exporté vers %s\n", csv_file);
        }

        free(tour);
    } else {
        fprintf(stderr, "Erreur lors du calcul de la tournée\n");
    }

    tsp_free_instance(inst);
    return 0;
}

