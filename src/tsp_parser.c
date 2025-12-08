#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tsp_types.h"
#include "distance.h"
#include "tsp_parser.h"

#define MAX_LINE_LENGTH 512
#define MAX_KEY_LENGTH   64
#define MAX_VALUE_LENGTH 256

// ---------------------------------------------------------------------
//  Fonctions utilitaires
// ---------------------------------------------------------------------

/**
 * Supprime les espaces et sauts de ligne à la fin d'une chaîne.
 */
static void remove_trailing_whitespace(char *str) {
    if (str == NULL) return;

    size_t len = strlen(str);
    while (len > 0) {
        unsigned char c = (unsigned char)str[len - 1];
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') {
            str[--len] = '\0';
        } else {
            break;
        }
    }
}

/**
 * Sépare une ligne en deux parties "clé : valeur".
 * Gère les formats "KEY : VALUE", "KEY:VALUE" et "KEY VALUE".
 */
static void extract_key_value(const char *line, char *key, size_t key_size, char *value, size_t value_size) {
    if (line == NULL || key == NULL || value == NULL) return;

    const char *colon = strchr(line, ':');
    if (colon == NULL) {
        // Cas "KEY VALUE"
        const char *space = strchr(line, ' ');
        if (space) {
            size_t key_len = (size_t)(space - line);
            if (key_len >= key_size) key_len = key_size - 1;
            strncpy(key, line, key_len);
            key[key_len] = '\0';

            // Ignore espaces avant la valeur
            while (*space && isspace((unsigned char)*space)) space++;
            strncpy(value, space, value_size - 1);
            value[value_size - 1] = '\0';
        } else {
            strncpy(key, line, key_size - 1);
            key[key_size - 1] = '\0';
            value[0] = '\0';
        }
        return;
    }

    // Cas standard "KEY : VALUE"
    size_t key_len = (size_t)(colon - line);
    if (key_len >= key_size) key_len = key_size - 1;
    strncpy(key, line, key_len);
    key[key_len] = '\0';

    const char *val_ptr = colon + 1;
    while (*val_ptr && isspace((unsigned char)*val_ptr)) val_ptr++;
    strncpy(value, val_ptr, value_size - 1);
    value[value_size - 1] = '\0';
    remove_trailing_whitespace(value);

    // Nettoyage de la clé
    remove_trailing_whitespace(key);
}

/**
 * Vérifie si une ligne commence par un mot clé donné (ignore la casse et les espaces).
 */
static int line_starts_with(const char *line, const char *prefix) {
    if (line == NULL || prefix == NULL) return 0;

    while (*line && isspace((unsigned char)*line)) line++;
    return strncasecmp(line, prefix, strlen(prefix)) == 0;
}

// ---------------------------------------------------------------------
//   Parsing principal
// ---------------------------------------------------------------------

TSP_Instance *tsp_read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur d’ouverture du fichier TSP");
        return NULL;
    }

    TSP_Instance *instance = (TSP_Instance *)calloc(1, sizeof(TSP_Instance));
    if (!instance) {
        fclose(file);
        fprintf(stderr, "Erreur d’allocation mémoire pour l’instance TSP.\n");
        return NULL;
    }
    instance->dist_type = DIST_UNKNOWN;

    char line[MAX_LINE_LENGTH];
    int reading_coords = 0;
    int coord_count = 0;

    while (fgets(line, sizeof(line), file)) {
        remove_trailing_whitespace(line);
        if (!*line) continue; // ligne vide → ignorer

        if (!reading_coords) {
            if (line_starts_with(line, "NAME")) {
                char key[MAX_KEY_LENGTH], value[MAX_VALUE_LENGTH];
                extract_key_value(line, key, sizeof(key), value, sizeof(value));
                strncpy(instance->name, value, sizeof(instance->name) - 1);

            } else if (line_starts_with(line, "COMMENT")) {
                char key[MAX_KEY_LENGTH], value[MAX_VALUE_LENGTH];
                extract_key_value(line, key, sizeof(key), value, sizeof(value));
                strncpy(instance->comment, value, sizeof(instance->comment) - 1);

            } else if (line_starts_with(line, "TYPE")) {
                char key[MAX_KEY_LENGTH], value[MAX_VALUE_LENGTH];
                extract_key_value(line, key, sizeof(key), value, sizeof(value));
                strncpy(instance->type, value, sizeof(instance->type) - 1);

            } else if (line_starts_with(line, "DIMENSION")) {
                char key[MAX_KEY_LENGTH], value[MAX_KEY_LENGTH];
                extract_key_value(line, key, sizeof(key), value, sizeof(value));
                remove_trailing_whitespace(value);
                instance->dimension = atoi(value);

            } else if (line_starts_with(line, "EDGE_WEIGHT_TYPE")) {
                char key[MAX_KEY_LENGTH], value[MAX_KEY_LENGTH];
                extract_key_value(line, key, sizeof(key), value, sizeof(value));
                instance->dist_type = parse_distance_type(value);

            } else if (line_starts_with(line, "NODE_COORD_SECTION")) {
                if (instance->dimension <= 0) {
                    fclose(file);
                    tsp_free_instance(instance);
                    fprintf(stderr, "Erreur : dimension invalide (%d)\n", instance->dimension);
                    return NULL;
                }

                reading_coords = 1;
                instance->x = malloc((size_t)instance->dimension * sizeof(double));
                instance->y = malloc((size_t)instance->dimension * sizeof(double));

                if (!instance->x || !instance->y) {
                    fclose(file);
                    tsp_free_instance(instance);
                    fprintf(stderr, "Erreur d’allocation mémoire pour les coordonnées.\n");
                    return NULL;
                }
            }

        } else { // Lecture des coordonnées
            if (line_starts_with(line, "EOF")) break;

            int id;
            double a, b;
            if (sscanf(line, "%d %lf %lf", &id, &a, &b) == 3) {
                if (id >= 1 && id <= instance->dimension) {
                    instance->x[id - 1] = a;
                    instance->y[id - 1] = b;
                    coord_count++;
                }
            }

            if (coord_count >= instance->dimension) break; // tout lu
        }
    }

    fclose(file);

    // Vérifications finales
    if (instance->dimension <= 0 || !instance->x || !instance->y) {
        tsp_free_instance(instance);
        fprintf(stderr, "Erreur : instance TSP incomplète.\n");
        return NULL;
    }

    if (instance->dist_type == DIST_UNKNOWN) {
        instance->dist_type = DIST_EUC_2D; // Valeur par défaut
    }

    // Calcul des distances
    build_distance_matrix(instance);
    return instance;
}

// ---------------------------------------------------------------------
//  Gestion mémoire & affichage
// ---------------------------------------------------------------------

void tsp_free_instance(TSP_Instance *inst) {
    if (!inst) return;
    free(inst->x);
    free(inst->y);
    free(inst->dist);
    free(inst);
}

void tsp_print_summary(const TSP_Instance *inst) {
    if (!inst) return;

    const char *dist_name =
        (inst->dist_type == DIST_EUC_2D) ? "EUC_2D" :
        (inst->dist_type == DIST_ATT)    ? "ATT" :
        (inst->dist_type == DIST_GEO)    ? "GEO" :
                                           "UNKNOWN";

    printf("=== TSP Instance ===\n");
    printf("Name: %s\n", inst->name);
    printf("Type: %s\n", inst->type);
    printf("Comment: %s\n", inst->comment);
    printf("Dimension: %d\n", inst->dimension);
    printf("EDGE_WEIGHT_TYPE: %s\n", dist_name);

    printf("First coords:\n");
    int n = inst->dimension;
    for (int i = 0; i < n && i < 5; ++i) {
        printf("  %d -> (%.6f, %.6f)\n", i + 1, inst->x[i], inst->y[i]);
    }

    if (n >= 2) {
        printf("Dist[1][2] = %.0f\n", inst->dist[0 * (size_t)n + 1]);
    }
}
