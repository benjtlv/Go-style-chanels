Channels
========

System programming project. Master degree's first year

This is a simple representation in C language, of chanels as implemented in the Go language. Used to make threads or Unix processes communicate between each other, in synchronous or asynchronous mode.

Examples:
--------

- Eratosthene sieve
- a ring of processes
- life game (graphical)
- Mandelbrot space (graphical) 

Compilation:
-----------

`make example`

Many use cases :

era [-l limit] [-n threads] [-s fifo size] (Eratosthene sieve with channels)
eratosthene (Eratosthene sieve without channels - for comparision)
ring [-c] (processes placed in ring . option c for the channels version)
life [-z frame size] [-n threads] [-s synchronous mode](life game)
mandelbrot [-n threads] [-s]

Manuel:
-------

Manual of the fifo data structure.

`man -M . channel`
`man -M . fifo`