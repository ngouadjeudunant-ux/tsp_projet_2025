#ifndef ALGO_BF_H
#define ALGO_BF_H

#include "tsp_parser.h"  // contient la définition de TSP_Instance (dont inst->n et inst->dist)

void bf_solve(const TSP_Instance *inst, int *best_tour, double *best_cost);

#endif /* ALGO_BF_H */
