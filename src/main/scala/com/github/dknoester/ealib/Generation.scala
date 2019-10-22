package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id

@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[FullReplace], name = "fullReplace")
//  new Type(value = classOf[FitnessProportionate], name = "fitnessProportionate"),
))
abstract class GenerationOperator extends ((Population, AbstractEvolutionaryAlgorithm) => Population) { }

class FullReplace extends GenerationOperator {
  override def apply(v1: Population, v2: AbstractEvolutionaryAlgorithm): Population = null
}
