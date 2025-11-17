#ifndef BRUTE_FORCE_H
#define BRUTE_FORCE_H


double brute(
    int nb_nodes,
    int nb_ressources,
    int *best_perm,
    unsigned long long *count_best,
    void *(*cout)(void *, int *)
);

#endif // BRUTE_FORCE_H
