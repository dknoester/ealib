package com.github.dknoester.ealib

import java.io.File

import com.fasterxml.jackson.databind.ObjectMapper
import org.rogach.scallop.ScallopConf


class Conf(arguments: Seq[String]) extends ScallopConf(arguments) {
  version("Scala EALib v0.1 (c) 2019 Dave Knoester")
  banner("Usage: ...")
  footer("\nfooter")

  val rngSeed = opt[Long](default=Some(System.currentTimeMillis()), descr="The random number generator seed.")
  val config = opt[File](descr="Path to a config file.", required=true)
  validateFileExists(config)

  verify()
}


trait CommandLineInterface {

  def registerSubtypes(mapper: ObjectMapper): Unit = {
    // Json.mapper.registerSubtypes(new NamedType(classOf[InDelMutation], "inDelMutation"))

  }

  def load(file: File): AbstractEvolutionaryAlgorithm = { null }

  def main(args: Array[String]) {
    val conf = new Conf(args)

    registerSubtypes(Json.mapper)

    load(conf.config.toOption.get).run()
  }
}


