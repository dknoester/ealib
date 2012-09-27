ealib
=====

Evolutionary Algorithms Library

This library contains a variety of generic components that are useful for
building evolutionary algorithms.  The focus of EALib is on compile-time
flexibility, as opposed to run-time flexibility.  EALib is thus ideally suited
for high-performance and/or large-scale evolutionary algorithms.  Moreover,
many of the components in EALib (e.g., selection strategies) are generic, and
can be reused freely in other libraries.

The following libraries are part of EALib:
    libea: 
        Core evolutionary algorithm components.
    libmkv:
        Markov network library.
    libcontrol:
        Control-based components (used with libmkv, useful for agent-based
        simulations).
    libfn:
        Contains an older version of Markov networks, a CUDA implementation of 
        Markov networks, and a partial implementation of artificial neural 
        networks.


Contributors:
David B. Knoester, Heather J. Goldsby, Randal S. Olson.
