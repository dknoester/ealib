package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonProperty, JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[FitnessProportionateSelection], name = "fitnessProportionateSelection"),
  new Type(value = classOf[TournamentSelection], name = "tournamentSelection"),
  new Type(value = classOf[RandomSelection], name = "randomSelection"),
))
abstract class SelectionOperator extends ((Int, Population, AbstractEvolutionaryAlgorithm) => Population) {
}

class FitnessProportionateSelection extends SelectionOperator {
  override def apply(n: Int, population: Population, ea: AbstractEvolutionaryAlgorithm): Population = null
}

class RandomSelection extends SelectionOperator {
  override def apply(n: Int, population: Population, ea: AbstractEvolutionaryAlgorithm): Population = null
}

class TournamentSelection(@JsonProperty(required=true) val r: Int,
                          @JsonProperty(required=true) val k: Int)
  extends SelectionOperator {

  override def apply(n: Int, population: Population, ea: AbstractEvolutionaryAlgorithm): Population = null
}
