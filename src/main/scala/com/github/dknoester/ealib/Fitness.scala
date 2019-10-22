package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id

trait Fitness { }

//class NumericFitness extends Fitness {
//  def value: Double
//
//  override def compare(x: Fitness, y: Fitness): Int =
//}
//
//
//class SingleValuedFitness extends Fitness  {
//  override def value: Double = ???
//
//  override def compareTo(that: Fitness): Int = ???
//
//  override def compare(x: Fitness, y: Fitness): Int = ???
//
//  override def compare(x: SingleValuedFitness, y: SingleValuedFitness): Int = ???
//}

@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[AllOnes], name = "allOnes")
))
abstract class FitnessFunction extends ((Individual, AbstractEvolutionaryAlgorithm) => Fitness) { }


class AllOnes() extends FitnessFunction {
  override def apply(v1: Individual, v2: AbstractEvolutionaryAlgorithm): Fitness = null
}

class BiologicalFitness() extends FitnessFunction {
  override def apply(v1: Individual, v2: AbstractEvolutionaryAlgorithm): Fitness = null
}