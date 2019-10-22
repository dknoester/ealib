package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonProperty, JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[RandomGenotype], name = "randomGenotype"),
))
abstract class PopulationGenerator extends (AbstractEvolutionaryAlgorithm => Population) {
}

class RandomGenotype(@JsonProperty(required=true) val size: Int) extends PopulationGenerator {
  override def apply(ea: AbstractEvolutionaryAlgorithm): Population = {
    new Population(List.tabulate(size)(n => ea.makeIndividual(ea.makeRandomGenotype())))
  }
}
