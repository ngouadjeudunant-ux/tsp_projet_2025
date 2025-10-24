#ifndef TSP_TYPES_H
#define TSP_TYPES_H

typedef enum {
    DIST_EUC_2D,
    DIST_ATT,
    DIST_GEO,
    DIST_UNKNOWN
} DistanceType;

typedef struct {
    char name[128];
    char comment[256];
    char type[32];
    int dimension;
    DistanceType dist_type;

    // Coordonnées "brutes" lues depuis TSPLIB (x,y en EUC/ATT ; lat,lon en GEO)
    double *x;   // taille = dimension
    double *y;   // taille = dimension

    // Matrice des distances (n x n) calculée selon EDGE_WEIGHT_TYPE
    double *dist; // taille = dimension * dimension
} TSP_Instance;

#endif
