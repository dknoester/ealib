package com.github.dknoester.ealib

import org.scalatest.FlatSpec

class ConfigurationTest extends FlatSpec {
  var json = """ {
        "evolutionaryAlgorithm": {
          "rng": {
            "seed": 1
          },
          "parentSelectionOperator": {
            "fitnessProportionateSelection": { }
          },
          "survivorSelectionOperator": {
            "tournamentSelection": { "r": 2, "k": 1 }
          },
         "populationGenerator": {
           "randomGenotype": { "size": 100 }
         }
        }
      }"""

  "An EvolutionaryAlgorithm" must "be configurable via deserialization" in {
    var ea = Json.deserialize[EvolutionaryAlgorithm](json)

    assert(ea.updateNumber == 0)
    assert(ea.parentSelectionOperator.isInstanceOf[FitnessProportionateSelection])
    assert(ea.survivorSelectionOperator.isInstanceOf[TournamentSelection])
  }

  it must "serialize" in {
    var ea = Json.deserialize[EvolutionaryAlgorithm](json)
    var s = Json.serialize(ea)

  }
}
