package com.github.dknoester

/** Provides classes for building and running evolutionary algorithms.
  *
  * EALib is meant to be an easily extensible library of classes that help with building and running evolutionary
  * algorithms for scientific and industrial research.  It also assists with data capture and replication.
  *
  * EALib uses the following definitions / abstractions:
  *
  * - [[ealib.Individual]] represent "solutions" in an evolutionary algorithm; in the case of
  * [[ealib.DigitalEvolution]], they are "organisms".
  *
  * - [[ealib.Genotype]] is the genetic code for an [[ealib.Individual]].  Genotypes change as a result of
  * recombination and mutation during reproduction.  Regardless of the underlying represenation, genotypes must
  * support iteration over their loci.
  *
  * - [[ealib.Phenotype]] is that characteristic of an [[ealib.Individual]] that is evaluated by a fitness function.
  * The phenotype may be equivalent to the genotype, or it may be produced by transforming the genotype into
  * something that *can* be evaluated (e.g., a genotype can be converted to a neural network).
  *
  * - [[ealib.Fitness]] is defined by the phenotype of the individual.  Biologically, fitness is the fecundity of the
  * individual's offspring, but in engineering, fitness is equivalent to a utility function, which we call the
  * [[ealib.FitnessFunction]].  Fitness is typically single-valued and decreasing, with an optimum at 0, however it
  * may also be multi-valued and/or increasing.  Regardless of cardinality and direction, an individual with "greater
  * fitness" should be preferred for selection & reproduction.
  *
  * - [[ealib.RecombinationOperator]] and [[ealib.MutationOperator]] are defined by the underlying genotype's
  * representation, and are used to recombine and mutate genotypes, respectively.
  *
  * - [[ealib.SelectionOperator]] pick individuals from populations for either reproduction or replacement.  Selection
  * usually prefers individuals with greater fitness.
  *
  * - [[ealib.Population]] is a collection of individuals.  While we frequently say that "individuals evolve," this
  * is not the case: The genotype of an individual is fixed over time, while the set of genotypes in a population
  * changes over time.
  *
  * - ([[ealib.DigitalEvolution]] only) [[ealib.Environment]] hold populations, and the (biological) fitness of an
  * individual depends on its interaction(s) with that environment.  The environment can be either simple or complex,
  * and may include interactions between individuals.
  */
package object ealib {
}
