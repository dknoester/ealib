package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.{JsonSubTypes, JsonTypeInfo, JsonTypeName}
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id


@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[PointMutation], name="pointMutation"),
  new Type(value = classOf[IndelMutation], name="indelMutation")
))
abstract class MutationOperator extends ((Genotype, AbstractEvolutionaryAlgorithm) => Genotype) { }

class PointMutation(val rate: Double) extends MutationOperator {
  override def apply(genotype: Genotype, ea: AbstractEvolutionaryAlgorithm): Genotype = genotype
}

class IndelMutation extends MutationOperator {
  override def apply(v1: Genotype, v2: AbstractEvolutionaryAlgorithm): Genotype = null
}