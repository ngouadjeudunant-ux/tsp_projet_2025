#ifndef ALGO_GA_H
#define ALGO_GA_H

#include "tsp_parser.h"  

int* ga_tour(const TSP_Instance *inst, int pop_size, int generations, double mutation_rate, int use_dpx);

#endif
