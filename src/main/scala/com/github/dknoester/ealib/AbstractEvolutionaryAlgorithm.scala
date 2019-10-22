package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[EvolutionaryAlgorithm], name = "evolutionaryAlgorithm"),
  new Type(value = classOf[DigitalEvolution], name = "digitalEvolution")
))
abstract class AbstractEvolutionaryAlgorithm(val rng: RandomNumberGenerator,
                                             val events: EventHandler,
                                             val generationOperator: GenerationOperator,
                                             val genotype: Genotype,
                                             val phenotype: Phenotype,
                                             val mutationOperator: MutationOperator,
                                             val recombinationOperator: RecombinationOperator,
                                             var population: Population,
                                             val populationGenerator: PopulationGenerator,
                                             val fitnessFunction: FitnessFunction) {

  var updateNumber: Int = 0

  def run(): Unit = {
    if(updateNumber == 0) {
      population = populationGenerator(this)
      events.populationGenerated(this)
    }
  }

  /**

  updates define time for an evolutionary algorithm.
   a single update *may* correspond to a population-wide generation, or it may be a small fraction of a generation.
    */

  def update(): Unit = {
    events.beforeUpdate(this)
    population = generationOperator(population, this)
    events.afterUpdate(this)
    updateNumber += 1
  }

  def calculateFitness(individual: Individual): Fitness = fitnessFunction(individual, this)
  def makeRandomGenotype(): Genotype = genotype.makeRandom(this)
  def makeMutatedGenotype(genotype: Genotype): Genotype = mutationOperator(genotype, this)
  def makePhenotype(genotype: Genotype): Phenotype = phenotype(genotype, this)
  def makeIndividual(genotype: Genotype): Individual = makeIndividual(genotype, None)
  def makeIndividual(genotype: Genotype, parents: Option[List[Individual]]): Individual = new Individual(genotype, parents, this)
}
