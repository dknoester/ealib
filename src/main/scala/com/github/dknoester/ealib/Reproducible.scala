package com.github.dknoester.ealib

/**
  * This trait is used to indicate that an object of type T can reproduce.
  *
  * @tparam T is the type that can reproduce.
  */
trait Reproducible[T] {

  /**
    * When called, this object will reproduce, triggering mutation and/or recombination, and return an object of the
    * same type.
    *
    * @param ea the enclosing ea.
    * @return the offspring.
    */
  def reproduce(ea: AbstractEvolutionaryAlgorithm): T
}
