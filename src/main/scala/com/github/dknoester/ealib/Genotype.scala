package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.{JsonProperty, JsonSubTypes, JsonTypeInfo, JsonValue}
import com.fasterxml.jackson.annotation.JsonSubTypes.Type
import com.fasterxml.jackson.annotation.JsonTypeInfo.Id

abstract class Locus {
  def mutate(ea: AbstractEvolutionaryAlgorithm): Locus
}

class BitLocus(@JsonValue val l: Int) extends Locus {
  override def mutate(ea: AbstractEvolutionaryAlgorithm): Locus = new BitLocus(ea.rng.nextBit())
}

@JsonTypeInfo(use = Id.NAME, include = JsonTypeInfo.As.WRAPPER_OBJECT)
@JsonSubTypes(Array(
  new Type(value = classOf[BitVector], name = "bitVector")
))
abstract class Genotype {
  def makeRandom(ea: AbstractEvolutionaryAlgorithm): Genotype
  def iterator: Iterator[Locus]
}

class BitVector(@JsonProperty(required=true) val length: Int,
                val genome: Array[BitLocus]) extends Genotype {

  override def makeRandom(ea: AbstractEvolutionaryAlgorithm): Genotype =
    new BitVector(length, Array.tabulate(length)(i => new BitLocus(ea.rng.nextBit())))

  override def iterator: Iterator[Locus] = genome.iterator
}