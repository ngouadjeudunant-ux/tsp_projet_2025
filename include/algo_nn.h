#ifndef ALGO_NN_H
#define ALGO_NN_H

#include "tsp_types.h"

int* nn_tour(const TSP_Instance *inst);             // retourne la tournée (avec retour au point de départ)
double tour_length(const TSP_Instance *inst, int *tour); // calcule la longueur totale

#endif
