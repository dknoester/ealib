package com.github.dknoester.ealib

import com.fasterxml.jackson.annotation.JsonProperty


class Population(val individuals: List[Individual]) extends Reproducible[Population] {

  def reproduce(ea: AbstractEvolutionaryAlgorithm): Population = {
    null
//    new Population(ea.recombine(1, this).map(g => new Individual(ea.mutate(g, ea), Some(individuals), ea)))
  }
}
