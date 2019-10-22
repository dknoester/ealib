package com.github.dknoester.ealib

class Individual(val genotype: Genotype,
                 val parents: Option[List[Individual]],
                 val ea: AbstractEvolutionaryAlgorithm) extends Reproducible[Individual] {

  lazy val phenotype: Phenotype = { ea.makePhenotype(genotype) }
  lazy val fitness: Fitness = { ea.calculateFitness(this) }

  def reproduce(ea: AbstractEvolutionaryAlgorithm): Individual = {
    ea.makeIndividual(ea.makeMutatedGenotype(genotype), Some(List(this)))
  }
//  **
//  *the generation of an individual is its depth within a lineage.
//  *the average generation of a population thus depends on what fraction of the population reproduces over time.
//  *we generally think of generation as a population-wide average.


}