#include <stdio.h>
#include <time.h>  // pour mesurer le temps ailleurs
#include "csv_export.h"


/**
 * @brief Exporte un résumé de la tournée au format CSV.
 *
 * @param filename   Nom du fichier de sortie.
 * @param instance   Nom de l’instance TSP.
 * @param method     Nom de l’algorithme utilisé (ex: "nn", "bf").
 * @param time_sec   Temps d'exécution (en secondes).
 * @param length     Longueur totale de la tournée.
 * @param tour       Tableau contenant la tournée (liste ordonnée des villes visitées).
 * @param n          Dimension (nombre de villes dans le problème).
 * @param entete     1 pour insérer l'entête (et écraser le fichier), 0 sinon
 *
 * @return 0 si succès, -1 si erreur à l'ouverture du fichier.
 */
int export_summary_csv(const char *filename, const char *instance, const char *method,
                       double time_sec, double length, const int *tour, int n, int entete) {
    FILE *fp = fopen(filename, entete ? "w" : "a"); // si pas d'entete, on fait append au fichier
    if (!fp) {
        perror("Erreur ouverture fichier CSV");
        return -1;
    }
    if (entete){
        fprintf(fp, "instance;méthode;durée(s);longueur;tournee\n");
    }
    fprintf(fp, "%s;%s;%.2f;%.0f;[", instance, method, time_sec, length);

    for (int i = 0; i <= n; ++i)
        fprintf(fp, "%d%s", tour[i] + 1, (i < n) ? "," : "");

    fprintf(fp, "]\n");
    fclose(fp);
    return 0;
}
