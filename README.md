Channels
========

Implémentation simple de channel à la go en C. Peuvent être utilisés de façons synchrones, entre processus lourds.

Arborescence:
-------------

racine : 
channel.c
channel.h
static_fifo.c
static_fifo.h
mandelbrot.c
era.c
eratosthene.c
life.c
life_game.c
life_game.h
ring.c
README.md
doc/
mann/

doc:
rapport.pdf

mann:
channel.n
fifo.n

Exemple:
--------

Crible d'Eratosthène, processus en anneaux (TP 3), jeu de la vie, espace de Mandelbrot. 

Compilation:
-----------

`make example`

plusieurs exemples possibles

era [-l limite] [-n nombre de threads] [-s taille de la file] (crible d'Eratosthene avec channels)
eratosthene (crible d'Eratosthene sans channels)
ring [-c] (processus en anneaux option c pour la version avec channel)
life [-z taille de la fenetre] [-n nombre de threads] [-s channels synchrones](jeu de la vie)
mandelbrot [-n nombre de threads] [-s]

Manuel:
-------

Pour le manuel de la structure de données fifo.

`man -M . channel`
`man -M . fifo`

Dépot GIT:
----------

http://moule.informatique.univ-paris-diderot.fr:8080/gonzalez/chanchanchan

