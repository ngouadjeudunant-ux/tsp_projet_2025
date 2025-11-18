// Compilation : gcc src/*.c -Iinclude -o tsp.exe -lm
// Execution exemple : ./tsp.exe -f tests/data/att15.tsp -m nn
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "tsp_types.h" 
#include "distance.h"
#include "tsp_parser.h"
#include "algo_nn.h"
#include "algo_bf.h"
#include "algo_rw.h"
#include "algo_2opt.h"
#include "algo_ga.h"
#include "csv_export.h"

// Variable globale pour la fonction coût
TSP_Instance *global_inst = NULL;

// Flag interruption Ctrl-C
volatile sig_atomic_t stop_requested = 0;

// Handler Ctrl-C
void interrupt_handler(int sig) {
    (void)sig;
    stop_requested = 1;
}

// Fonction de coût BF
void *tsp_cost(void *unused, int *perm) {
    (void)unused;
    int n = global_inst->dimension;
    unsigned long long *cost = malloc(sizeof(unsigned long long));
    if (!cost) return NULL;

    *cost = 0;
    for (int i = 0; i < n - 1; ++i)
        *cost += (unsigned long long)(global_inst->dist[perm[i] * n + perm[i + 1]]);
    *cost += (unsigned long long)(global_inst->dist[perm[n - 1] * n + perm[0]]);

    return cost;
}

void usage(const char *prog) {
    printf("Usage : %s -f <fichier.tsp> -m <nn|bf|rw|nn2opt|rw2opt|ga> "
           "[ga: pop gen mut] [-o <export.csv>]\n", prog);
}

// Fonction de test des distances. 
void test_distance_calculation() {
    printf("\n=== TEST DE CALCUL DES DISTANCES TSPLIB ===\n");
    
    // 1. Définition de l'Instance de Test (V1: 10, 20 ; V2: 14, 23)
    TSP_Instance test_instance;
    test_instance.dimension = 2;
    test_instance.x = malloc(2 * sizeof(double));
    test_instance.y = malloc(2 * sizeof(double));
    test_instance.dist = NULL; 

    test_instance.x[0] = 10.0;
    test_instance.y[0] = 20.0;
    test_instance.x[1] = 14.0;
    test_instance.y[1] = 23.0;

    // --- TEST EUC_2D ---
    test_instance.dist_type = DIST_EUC_2D;
    build_distance_matrix(&test_instance);
    double dist_euc = test_instance.dist[0 * 2 + 1];
    printf("EUC_2D (V1:10,20; V2:14,23) : %.0f\n", dist_euc);
    if (test_instance.dist) free(test_instance.dist); 
    test_instance.dist = NULL;

    // --- TEST ATT ---
    test_instance.dist_type = DIST_ATT;
    build_distance_matrix(&test_instance);
    double dist_att = test_instance.dist[0 * 2 + 1];
    printf("ATT    (V1:10,20; V2:14,23) : %.0f\n", dist_att);
    if (test_instance.dist) free(test_instance.dist);
    test_instance.dist = NULL;

    // --- TEST GEO ---
    // Utilisation de coordonnées simplifiées pour un test fonctionnel
    test_instance.x[0] = 40.0; 
    test_instance.y[0] = 5.0;  
    test_instance.x[1] = 40.0; 
    test_instance.y[1] = 5.01; 

    test_instance.dist_type = DIST_GEO;
    build_distance_matrix(&test_instance);
    double dist_geo = test_instance.dist[0 * 2 + 1];
    printf("GEO    (V1:40.0, 5.0; V2:40.0, 5.01) : %.0f\n", dist_geo);
    if (test_instance.dist) free(test_instance.dist);
    test_instance.dist = NULL;
    
    // Libération des coordonnées
    free(test_instance.x);
    free(test_instance.y);

    printf("=======================================\n");
}

int main(int argc, char **argv) {

    test_distance_calculation(); 

    const char *fichier = NULL;
    const char *methode = NULL;
    const char *csv_file = NULL;

    // paramètres GA
    int pop_size = 50;
    int generations = 200;
    double mut_rate = 0.05;

    // Lecture arguments
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-f") && i + 1 < argc)
            fichier = argv[++i];

        else if (!strcmp(argv[i], "-m") && i + 1 < argc) {
            methode = argv[++i];
            if (!strcmp(methode, "ga")) {
                if (argc < i + 4) {
                    printf("Usage GA : -m ga <pop> <gen> <mut>\n");
                    return 1;
                }
                pop_size = atoi(argv[++i]);
                generations = atoi(argv[++i]);
                mut_rate = atof(argv[++i]);
            }
        }

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

    // Installation du handler Ctrl-C
    signal(SIGINT, interrupt_handler);

    // Lecture instance
    TSP_Instance *inst = tsp_read_file(fichier);
    if (!inst) {
        fprintf(stderr, "Erreur lecture fichier.\n");
        return 2;
    }

    global_inst = inst;

    int *tour = NULL;
    double length = 0.0;
    clock_t start = clock();

    // --- Méthodes ---
    if (!strcmp(methode, "nn")) {
        tour = nn_tour(inst);
        if (tour) length = tour_length(inst, tour);

    } else if (!strcmp(methode, "rw")) {
        tour = rw_tour(inst);
        if (tour) length = tour_length(inst, tour);

    } else if (!strcmp(methode, "nn2opt")) {
        tour = nn_tour(inst);
        if (tour) {
            improve_2opt(inst, tour);
            length = tour_length(inst, tour);
        }

    } else if (!strcmp(methode, "rw2opt")) {
        tour = rw_tour(inst);
        if (tour) {
            improve_2opt(inst, tour);
            length = tour_length(inst, tour);
        }

    } else if (!strcmp(methode, "bf")) {
        tour = malloc(inst->dimension * sizeof(int));
        if (tour) {
            unsigned long long best_cost;
            brute(inst->dimension, 0, tour, &best_cost, tsp_cost);
            length = (double)best_cost;
        }

    } else if (!strcmp(methode, "ga")) {
        tour = ga_tour(inst, pop_size, generations, mut_rate);
        if (tour) length = tour_length(inst, tour);

    } else {
        printf("Méthode inconnue.\n");
        return 3;
    }

    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;

    // --- Affichage ---
    if (stop_requested)
        printf("\n[!] Interruption utilisateur (Ctrl-C)\n");

    if (tour) {
        printf("[!] Meilleure solution trouvée :\n");
        printf("Méthode : %s\n", methode);

        printf("Tournée : ");
        for (int i = 0; i < inst->dimension; ++i)
            printf("%d ", tour[i] + 1);
        printf("%d\n", tour[0] + 1);

        printf("Longueur : %.0f\n", length);
        printf("Durée    : %.3fs\n", elapsed);

        if (csv_file)
            export_summary_csv(csv_file, inst->name, methode, elapsed, length, tour, inst->dimension);

        free(tour);
    }

    tsp_free_instance(inst);
    return 0;
}
