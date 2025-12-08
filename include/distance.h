#ifndef DISTANCE_H
#define DISTANCE_H

#include "tsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Déduit le DistanceType depuis la chaîne EDGE_WEIGHT_TYPE
DistanceType parse_distance_type(const char *s);

// Remplit inst->dist (dimension x dimension) à partir de inst->x, inst->y
// en fonction de inst->dist_type (EUC_2D, ATT, GEO).
void build_distance_matrix(TSP_Instance *inst);

#ifdef __cplusplus
}
#endif

#endif
