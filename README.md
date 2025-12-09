# Projet TSP

## Membres de l’équipe

- GIHALU Russel  
- TONFACK Dunant  
- GAHA Wassim  

## Comment compiler

### Sous Linux

Le projet inclut un `Makefile`. Pour compiler et générer l’exécutable :

- Commande : `make`  
- Binaire généré : `bin/tsp`  

### Sous Windows

Compilation manuelle avec GCC :

- Commande : `gcc src/*.c -Iinclude -o bin/tsp.exe -lm`  

## Usage

Exécution générale :

- Sous Linux : `./bin/tsp [options]`  
- Sous Windows : `bin/tsp.exe [options]`  

## Structure du projet

- `src/` : fichiers source C (`main.c`, `algo_nn.c`, `algo_rw.c`, `algo_2opt.c`, `algo_ga.c`, etc.)  
- `include/` : fichiers d’en-tête (`*.h`)  
- `bin/` : répertoire de sortie pour l’exécutable (`tsp` ou `tsp.exe`)  
- `Makefile` : règles de compilation sous Linux  
- éventuellement `tests/data/` : fichiers TSPLIB à utiliser avec l’option `-f`.
