package com.github.dknoester.ealib

import org.scalatest.FlatSpec

class RandomNumberGeneratorTest extends FlatSpec {

  "A RandomNumberGenerator" must "be serializable" in {
    var s = Json.serialize(new RandomNumberGenerator(1))
    assert(s.contains(""""seed":1,"state":"""))
  }

  it must "support round-trip ser/des and remain consistent" in {
    var rng = new RandomNumberGenerator(1)
    var s = Json.serialize(rng)
    var rngTwo = Json.deserialize[RandomNumberGenerator](s)

    for(i <- 0 to 10) {
      assert(rng.nextInt() == rngTwo.nextInt())
    }
  }

}
