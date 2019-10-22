package com.github.dknoester.ealib

import org.scalatest.FlatSpec

class EventHandlerTest extends FlatSpec {
  var json = """ {
        "evolutionaryAlgorithm": {
          "rng": { "seed": 1 },
          "events": { "defaultEvents": { } },
          "parentSelectionOperator": { "fitnessProportionateSelection": { } },
          "survivorSelectionOperator": { "tournamentSelection": { "r": 2, "k": 1 } },
          "populationGenerator": { "randomGenotype": { "size": 100 } }
        }
      }"""

  trait MyEventHandler extends DefaultEvents {
    override def beforeUpdate(ea: AbstractEvolutionaryAlgorithm): Unit = println("bar")
  }

  "Evolutionary algorithms" must "be extensible with custom event handlers" in {
    var ea = Json.deserialize[AbstractEvolutionaryAlgorithm](json)
    ea.events.beforeUpdate(ea)


  }
}
