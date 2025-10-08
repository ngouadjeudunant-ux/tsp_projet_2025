/* signal_ctrlc.c
 * Handler SIGINT (Ctrl-C) : intercepte l'interruption et exécute actions sûres.
 * Comportement : afficher la meilleure tournée courante, proposer reprendre/quitter.
 * Doit être initialisé par main (ex: install_signal_handler(&state)).
 */
