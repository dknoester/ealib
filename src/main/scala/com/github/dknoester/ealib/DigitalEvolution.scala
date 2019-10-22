package com.github.dknoester.ealib

class DigitalEvolution(rng: RandomNumberGenerator,
                       events: DefaultEvents,
                       generationOperator: GenerationOperator,
                       genotype: Genotype,
                       phenotypeMappingOperator: Phenotype,
                       mutationOperator: MutationOperator,
                       recombinationOperator: RecombinationOperator,
                       population: Population,
                       populationGenerator: PopulationGenerator,
                       val environment: Environment)
  extends AbstractEvolutionaryAlgorithm(
          rng,
          events,
          generationOperator,
          genotype,
          phenotypeMappingOperator,
          mutationOperator,
          recombinationOperator,
          population,
          populationGenerator,
          new BiologicalFitness) {
}
