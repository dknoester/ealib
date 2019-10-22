package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[Crossover], name = "crossover"),
))
abstract class RecombinationOperator extends ((Int, Population, AbstractEvolutionaryAlgorithm) => List[Genotype]) { }

class Crossover extends RecombinationOperator {
  override def apply(v1: Int, v2: Population, v3: AbstractEvolutionaryAlgorithm): List[Genotype] = v2.individuals.map(i => i.genotype)
}
