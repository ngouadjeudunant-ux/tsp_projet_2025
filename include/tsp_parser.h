/* tsp_parser.h
 * Déclare parse_tsp(const char*, Instance*) et helpers du parseur.
 * Fournit le contrat attendu par les autres modules (allocation de coord).
 */

#ifndef TSP_PARSER_H
#define TSP_PARSER_H

#include "tsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Lecture d'un fichier .tsp (TSPLIB) qui contient NODE_COORD_SECTION
// - alloue et remplit TSP_Instance
// - calcule la matrice des distances selon EDGE_WEIGHT_TYPE (EUC_2D, ATT, GEO)
TSP_Instance *tsp_read_file(const char *filename);

// Libération mémoire
void tsp_free_instance(TSP_Instance *inst);

// Affiche un résumé (debug)
void tsp_print_summary(const TSP_Instance *inst);

#ifdef __cplusplus
}
#endif

#endif

