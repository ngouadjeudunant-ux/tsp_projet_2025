#ifndef ALGO_RW_H
#define ALGO_RW_H

#include "tsp_types.h"

int* rw_tour(const TSP_Instance *inst);
void int_array_pop(int ** array, int index, int length);

#endif