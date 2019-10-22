package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[DefaultEvents], name = "defaultEvents")
))
abstract class EventHandler {
  def populationGenerated(ea: AbstractEvolutionaryAlgorithm): Unit
  def beforeUpdate(ea: AbstractEvolutionaryAlgorithm): Unit
  def afterUpdate(ea: AbstractEvolutionaryAlgorithm): Unit
  def fitnessEvaluated(individual: Individual, ea: AbstractEvolutionaryAlgorithm): Unit
}

class DefaultEvents extends EventHandler {
  override def populationGenerated(ea: AbstractEvolutionaryAlgorithm): Unit = { }
  override def beforeUpdate(ea: AbstractEvolutionaryAlgorithm): Unit = { }
  override def afterUpdate(ea: AbstractEvolutionaryAlgorithm): Unit = { }
  override def fitnessEvaluated(individual: Individual, ea: AbstractEvolutionaryAlgorithm): Unit = { }
}