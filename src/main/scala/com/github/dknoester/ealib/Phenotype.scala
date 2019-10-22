package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[NeuralNetwork], name = "neuralNetwork"),
))
abstract class Phenotype extends ((Genotype, AbstractEvolutionaryAlgorithm) => Phenotype) { }

class NeuralNetwork extends Phenotype {
  override def apply(v1: Genotype, v2: AbstractEvolutionaryAlgorithm): Phenotype = null
}
