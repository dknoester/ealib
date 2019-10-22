package com.github.dknoester.ealib

import org.scalatest.FlatSpec

//import org.scalatest.FunSuite
//package ealib
//
//import org.scalatest.FlatSpec
//
//class ConfigurationTest extends FlatSpec {
//
//
//  "An EvolutionaryAlgorithm" must "be configurable via deserialization" in {
//    var ea = Json.deserialize[EvolutionaryAlgorithm](json)
//
//    assert(ea.updateNumber == 0)
//    assert(ea.parentSelectionOperator.isInstanceOf[FitnessProportionateSelection])
//    assert(ea.survivorSelectionOperator.isInstanceOf[TournamentSelection])
//  }
//
//  it must "serialize" in {
//    var ea = Json.deserialize[EvolutionaryAlgorithm](json)
//    var s = Json.serialize(ea)
//
//  }
//}

//
//trait AbstractBar {
//  def baz(): Unit = { }
//}
//
//class Foo extends AbstractBar {
//  def foo(): Unit = baz()
//}
//
//trait Bar extends AbstractBar {
//  override def baz(): Unit = { println("baz") }
//}



class MutationTest extends FlatSpec {

  var json = """{
        "evolutionaryAlgorithm": {
         "rng": { "seed": 1 },
          "mutationOperator": { "pointMutation": { } },
          "genotype": { "bitVector": { "length": 100 } },
          "populationGenerator": { "randomGenotype": { "size": 100 } }
        }
      }"""

  "Mutation operators" must "be able to iterate over genotypes" in {
    var ea = Json.deserialize[AbstractEvolutionaryAlgorithm](json)
    var g = ea.genotype.makeRandom(ea)

    g.iterator.next().mutate(ea)


    var s = Json.serialize(g)
    assert(s == "")
  }
//
//  test("traits") {
//    var f = new Foo with Bar
//    f.foo()
//  }
//
//  test("that point mutation serdes is correct") {
//    val json = """{"pointMutation":{"rate":0.5}}"""
//    assert(Json.serialize(new PointMutation(0.5)) == json)
//    assert(Json.deserialize[PointMutation](json).rate == 0.5)
//  }
//
//  test("that serdes is polymorphic") {
//    val json = """{"inDelMutation":{}}"""
//    assert(Json.deserialize[MutationOperator](json) != null)
//  }

}
