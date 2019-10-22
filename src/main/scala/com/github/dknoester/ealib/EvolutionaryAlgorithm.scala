package com.github.dknoester.ealib

class EvolutionaryAlgorithm(rng: RandomNumberGenerator,
                            events: DefaultEvents,
                            generationOperator: GenerationOperator,
                            genotype: Genotype,
                            phenotypeMappingOperator: Phenotype,
                            mutationOperator: MutationOperator,
                            recombinationOperator: RecombinationOperator,
                            population: Population,
                            populationGenerator: PopulationGenerator,
                            fitnessFunction: FitnessFunction,
                            val parentSelectionOperator: SelectionOperator,
                            val survivorSelectionOperator: SelectionOperator)
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
          fitnessFunction) {
}
