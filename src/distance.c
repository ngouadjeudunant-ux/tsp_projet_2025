
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "distance.h"

// ---------- helpers ----------

static inline double round_nearest(double v) {
    return floor(v + 0.5);
}

// ATT (pseudo-euclidienne) — définition TSPLIB
// r = sqrt( ((dx)^2 + (dy)^2) / 10.0 )
// t = (int) r
// d = (t < r) ? t + 1 : t
static inline int dist_att_ij(double xi, double yi, double xj, double yj) {
    double dx = xi - xj;
    double dy = yi - yj;
    double r = sqrt((dx*dx + dy*dy) / 10.0);
    int t = (int) r;
    return (t < r) ? (t + 1) : t;
}

// EUC_2D — distance euclidienne, arrondie à l’entier le plus proche
static inline int dist_euc2d_ij(double xi, double yi, double xj, double yj) {
    double dx = xi - xj;
    double dy = yi - yj;
    return (int) round_nearest(sqrt(dx*dx + dy*dy));
}

// Conversion GEO TSPLIB : coord est fournie sous la forme DD.MM (minutes = partie décimale)
// lat/long en radians : PI * (deg + 5.0 * min / 3.0) / 180.0
static void geo_tsplib_to_radians(double coord_deg_min, double *out_rad) {
    double deg = floor(coord_deg_min);
    double min = coord_deg_min - deg;
    double val = deg + (5.0 * min) / 3.0;
    *out_rad = M_PI * val / 180.0;
}

// Distance GEO TSPLIB (rayon = 6378.388 km, arrondi entier)
// d = int( RRR * arccos( 0.5 * [ (1+q1)*q2 - (1-q1)*q3 ] ) + 1.0 )
// q1 = cos(lon_i - lon_j)
// q2 = cos(lat_i - lat_j)
// q3 = cos(lat_i + lat_j)
static inline int dist_geo_ij(double lati_degmin, double loni_degmin,
                              double latj_degmin, double lonj_degmin) {
    const double RRR = 6378.388;
    double lati, loni, latj, lonj;
    geo_tsplib_to_radians(lati_degmin, &lati);
    geo_tsplib_to_radians(loni_degmin, &loni);
    geo_tsplib_to_radians(latj_degmin, &latj);
    geo_tsplib_to_radians(lonj_degmin, &lonj);

    double q1 = cos(loni - lonj);
    double q2 = cos(lati - latj);
    double q3 = cos(lati + latj);
    double val = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3);

    // clamp numérique (précautions dues aux arrondis flottants)
    if (val > 1.0) val = 1.0;
    if (val < -1.0) val = -1.0;

    return (int)(RRR * acos(val) + 1.0);
}

// ---------- API ----------

DistanceType parse_distance_type(const char *s) {
    if (!s) return DIST_UNKNOWN;
    if (strcmp(s, "EUC_2D") == 0) return DIST_EUC_2D;
    if (strcmp(s, "ATT")    == 0) return DIST_ATT;
    if (strcmp(s, "GEO")    == 0) return DIST_GEO;
    return DIST_UNKNOWN;
}

void build_distance_matrix(TSP_Instance *inst) {
    int n = inst->dimension;
    if (!inst || n <= 0 || !inst->x || !inst->y) return;

    inst->dist = (double*)malloc((size_t)n * (size_t)n * sizeof(double));
    for (int i = 0; i < n; ++i) {
        inst->dist[i*(size_t)n + i] = 0.0;
        for (int j = i + 1; j < n; ++j) {
            int dij = 0;
            switch (inst->dist_type) {
                case DIST_EUC_2D:
                    dij = dist_euc2d_ij(inst->x[i], inst->y[i], inst->x[j], inst->y[j]);
                    break;
                case DIST_ATT:
                    dij = dist_att_ij(inst->x[i], inst->y[i], inst->x[j], inst->y[j]);
                    break;
                case DIST_GEO:
                    // En GEO, x = latitude, y = longitude telles que fournies par le fichier TSPLIB
                    dij = dist_geo_ij(inst->x[i], inst->y[i], inst->x[j], inst->y[j]);
                    break;
                default:
                    dij = 0;
            }
            inst->dist[i*(size_t)n + j] = (double)dij;
            inst->dist[j*(size_t)n + i] = (double)dij; // symétrique
        }
    }
}
