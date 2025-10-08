/* parser.c
 * Lit un fichier TSPLIB (NAME, DIMENSION, EDGE_WEIGHT_TYPE, NODE_COORD_SECTION).
 * Remplit une Instance (nom, n, kind, coord) allouée dynamiquement.
 * Entrée : chemin fichier ; Sortie : Instance via pointeur (0 = OK, -1 = erreur).
 */
