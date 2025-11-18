// Compilation : gcc src/*.c -Iinclude -o tsp.exe -lm
// Execution exemple : ./tsp.exe -f tests/data/att15.tsp -m nn
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tsp_parser.h"
#include "algo_nn.h"
#include "algo_bf.h"
#include "algo_rw.h"
#include "csv_export.h"

// Variable globale utilisée dans tsp_cost
TSP_Instance *global_inst = NULL;

// Fonction de coût pour le TSP
void *tsp_cost(void *unused, int *perm) {
    (void)unused;
    int n = global_inst->dimension;
    unsigned long long *cost = malloc(sizeof(unsigned long long));
    *cost = 0;

    for (int i = 0; i < n - 1; ++i)
        *cost += (unsigned long long)(global_inst->dist[perm[i] * n + perm[i + 1]]);
    *cost += (unsigned long long)(global_inst->dist[perm[n - 1] * n + perm[0]]);
    return cost;
}

void usage(const char *prog) {
    printf("Usage : %s -f <fichier.tsp> -m <nn|bf> [-o <export.csv>]\n", prog);
}

int main(int argc, char **argv) {
    const char *fichier = NULL;
    const char *methode = NULL;
    const char *csv_file = NULL;

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

    TSP_Instance *inst = tsp_read_file(fichier);
    if (!inst) {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", fichier);
        return 2;
    }

    global_inst = inst; // rendre l'instance accessible à tsp_cost

    int *tour = NULL;
    double length = 0.0;
    clock_t start = clock();

    if (!strcmp(methode, "nn")) {
        tour = nn_tour(inst);
        if (tour)
            length = tour_length(inst, tour);
    } else if (!strcmp(methode, "bf")) {
        tour = malloc(inst->dimension * sizeof(int));
        if (tour) {
            unsigned long long best_cost;
            brute(inst->dimension, 0, tour, &best_cost, tsp_cost);
            length = (double)best_cost;
        }
    } else if (!strcmp(methode, "rw")){
        tour = rw_tour(inst);
        if (tour)
            length = tour_length(inst, tour);
    } else {
        fprintf(stderr, "Méthode inconnue : %s\n", methode);
        usage(argv[0]);
        tsp_free_instance(inst);
        return 3;
    }

    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;

    if (tour) {
        printf("Méthode : %s\n", methode);
        printf("Tournée : ");
        for (int i = 0; i < inst->dimension; ++i)
            printf("%d ", tour[i] + 1);
        printf("%d\n", tour[0] + 1); // retour à la ville de départ

        printf("Longueur : %.0f\nDurée : %.3fs\n", length, elapsed);

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
