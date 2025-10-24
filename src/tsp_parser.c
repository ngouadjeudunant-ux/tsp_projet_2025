
/* parser.c
 * Lit un fichier TSPLIB (NAME, DIMENSION, EDGE_WEIGHT_TYPE, NODE_COORD_SECTION).
 * Remplit une Instance (nom, n, kind, coord) allouée dynamiquement.
 * Entrée : chemin fichier ; Sortie : Instance via pointeur (0 = OK, -1 = erreur).
 */

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tsp_types.h"
#include "distance.h"
#include "tsp_parser.h"

#define LBUFSZ 512

static void trim_trailing(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n && (s[n-1] == '\r' || s[n-1] == '\n' || isspace((unsigned char)s[n-1]))) {
        s[--n] = '\0';
    }
}

static void parse_kv(char *line, char *key, size_t ksz, char *val, size_t vsz) {
    // supporte "KEY : value" et "KEY: value"
    char *colon = strchr(line, ':');
    if (!colon) {
        // parfois c'est "KEY value" (rare) — on tente un split premier espace
        char *sp = strchr(line, ' ');
        if (sp) {
            size_t klen = (size_t)(sp - line);
            if (klen >= ksz) klen = ksz - 1;
            strncpy(key, line, klen);
            key[klen] = '\0';
            while (*sp && isspace((unsigned char)*sp)) sp++;
            strncpy(val, sp, vsz-1);
            val[vsz-1] = '\0';
        } else {
            strncpy(key, line, ksz-1);
            key[ksz-1] = '\0';
            val[0] = '\0';
        }
        return;
    }

    // key = [start .. colon)
    size_t klen = (size_t)(colon - line);
    if (klen >= ksz) klen = ksz - 1;
    strncpy(key, line, klen);
    key[klen] = '\0';

    // val = colon+1 .. end
    const char *p = colon + 1;
    while (*p && isspace((unsigned char)*p)) p++;
    strncpy(val, p, vsz - 1);
    val[vsz - 1] = '\0';
    trim_trailing(val);

    // trim key trailing spaces
    size_t kk = strlen(key);
    while (kk && isspace((unsigned char)key[kk-1])) key[--kk] = '\0';
}

static int starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

TSP_Instance *tsp_read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("open tsp");
        return NULL;
    }

    TSP_Instance *inst = (TSP_Instance*)calloc(1, sizeof(TSP_Instance));
    inst->dist_type = DIST_UNKNOWN;

    char line[LBUFSZ];
    int in_coords = 0;
    int coord_idx = 0;

    while (fgets(line, sizeof(line), f)) {
        trim_trailing(line);
        if (!*line) continue;

        if (!in_coords) {
            if (starts_with(line, "NAME")) {
                char k[64], v[256];
                parse_kv(line, k, sizeof(k), v, sizeof(v));
                strncpy(inst->name, v, sizeof(inst->name)-1);
            } else if (starts_with(line, "COMMENT")) {
                char k[64], v[256];
                parse_kv(line, k, sizeof(k), v, sizeof(v));
                strncpy(inst->comment, v, sizeof(inst->comment)-1);
            } else if (starts_with(line, "TYPE")) {
                char k[64], v[64];
                parse_kv(line, k, sizeof(k), v, sizeof(v));
                strncpy(inst->type, v, sizeof(inst->type)-1);
            } else if (starts_with(line, "DIMENSION")) {
                char k[64], v[64];
                parse_kv(line, k, sizeof(k), v, sizeof(v));
                inst->dimension = atoi(v);
            } else if (starts_with(line, "EDGE_WEIGHT_TYPE")) {
                char k[64], v[64];
                parse_kv(line, k, sizeof(k), v, sizeof(v));
                inst->dist_type = parse_distance_type(v);
            } else if (starts_with(line, "NODE_COORD_SECTION")) {
                if (inst->dimension <= 0) {
                    fclose(f);
                    tsp_free_instance(inst);
                    return NULL;
                }
                in_coords = 1;
                inst->x = (double*)malloc((size_t)inst->dimension * sizeof(double));
                inst->y = (double*)malloc((size_t)inst->dimension * sizeof(double));
            }
            // Ignorer le reste (DISPLAY_DATA_SECTION, etc.) pour le socle
        } else {
            if (starts_with(line, "EOF")) break;

            // format: id  val1  val2
            // EUC/ATT : (id, x, y)
            // GEO : (id, latitude, longitude) — au format DD.MM (minutes = partie décimale)
            int id;
            double a, b;
            if (sscanf(line, "%d %lf %lf", &id, &a, &b) == 3) {
                if (id >= 1 && id <= inst->dimension) {
                    inst->x[id-1] = a;
                    inst->y[id-1] = b;
                    coord_idx++;
                }
            }
            if (coord_idx >= inst->dimension) {
                // on a tout lu, le reste (EOF/sections annexes) peut être ignoré
                break;
            }
        }
    }

    fclose(f);

    // garde-fous
    if (inst->dimension <= 0 || !inst->x || !inst->y) {
        tsp_free_instance(inst);
        return NULL;
    }

    if (inst->dist_type == DIST_UNKNOWN) {
        // Par défaut, beaucoup d’instances CC2 utilisent EUC_2D/ATT/GEO ; on force EUC_2D si non précisé
        inst->dist_type = DIST_EUC_2D;
    }

    // construire la matrice des distances selon le type
    build_distance_matrix(inst);
    return inst;
}

void tsp_free_instance(TSP_Instance *inst) {
    if (!inst) return;
    free(inst->x);
    free(inst->y);
    free(inst->dist);
    free(inst);
}

void tsp_print_summary(const TSP_Instance *inst) {
    if (!inst) return;
    const char *t =
        (inst->dist_type == DIST_EUC_2D) ? "EUC_2D" :
        (inst->dist_type == DIST_ATT)    ? "ATT" :
        (inst->dist_type == DIST_GEO)    ? "GEO" : "UNKNOWN";

    printf("=== TSP Instance ===\n");
    printf("Name: %s\n", inst->name);
    printf("Type: %s\n", inst->type);
    printf("Comment: %s\n", inst->comment);
    printf("Dimension: %d\n", inst->dimension);
    printf("EDGE_WEIGHT_TYPE: %s\n", t);

    int n = inst->dimension;
    printf("First coords:\n");
    for (int i = 0; i < n && i < 5; ++i) {
        printf("  %d -> (%.6f, %.6f)\n", i+1, inst->x[i], inst->y[i]);
    }
    printf("Dist[1][2] = %.0f\n", (n>=2) ? inst->dist[0*(size_t)n + 1] : 0.0);
}
*/

/* =====================================================================
 *  tsp_parser.c
 *  ------------------
 *  Lecture et parsing de fichiers TSPLIB (.tsp)
 *  Remplit une structure TSP_Instance à partir d'un fichier.
 *
 *  Étapes :
 *    1. Lecture des en-têtes : NAME, COMMENT, TYPE, DIMENSION, EDGE_WEIGHT_TYPE
 *    2. Lecture des coordonnées (NODE_COORD_SECTION)
 *    3. Construction automatique de la matrice de distances
 * ===================================================================== */

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
//  🔹  Fonctions utilitaires
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
//  🔹  Parsing principal
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
//  🔹  Gestion mémoire & affichage
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
