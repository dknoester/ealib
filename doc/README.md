# EALib: An Evolutionary Algorithms Library #

EALib is a series of C++ libraries containing a variety of generic components that are useful for building evolutionary algorithms.  The focus of EALib is on compile-time flexibility, as opposed to run-time flexibility.  EALib is thus ideally suited for high-performance and/or large-scale evolutionary algorithms.  Moreover, many of the components in EALib (e.g., selection strategies) are generic, and can be easily reused as building blocks for more elaborate evolutionary algorithms.  

The following libraries are the main parts of EALib:

- **libea**:  Evolutionary algorithm library.
Contains components related to various kinds of evolutionary algorithms, as well as more support-related functionality such as checkpointing, parameters, command-line interfaces, and the like.  

- **libmkv**: Markov network library.
Contains an implementation of Markov networks and EALib-compatible components to enable their evolution.

- **libnn**: Artificial neural network library.
Contains an implementation of Artificial Neural Networks (feedforward, recurrent, and continuous-time recurrent).  *(In progress.)*

## Installation
See INSTALL.md *(In progress.)*

## Examples and User's guide
See doc/users-guide.html *(In progress.)*
