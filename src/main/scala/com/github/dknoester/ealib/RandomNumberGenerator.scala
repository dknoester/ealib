package com.github.dknoester.ealib

import java.io.{ByteArrayInputStream, ByteArrayOutputStream, ObjectInputStream, ObjectOutputStream}
import java.util.Base64

import com.fasterxml.jackson.core.{JsonGenerator, JsonParser}
import com.fasterxml.jackson.databind.annotation.{JsonDeserialize, JsonSerialize}
import com.fasterxml.jackson.databind._
import org.apache.commons.math3.random.RandomDataGenerator

@JsonSerialize(using = classOf[RandomNumberGeneratorSerializer])
@JsonDeserialize(using = classOf[RandomNumberGeneratorDeserializer])
class RandomNumberGenerator(val seed: Long) {

  var randomData = new RandomDataGenerator
  randomData.reSeed(seed)

  def this(rdg: RandomDataGenerator) {
    this(1)
    randomData = rdg
  }

  def nextInt(): Int = randomData.nextInt(Int.MinValue, Int.MaxValue)

  def nextBit(): Int = randomData.nextInt(0, 1)
}

class RandomNumberGeneratorSerializer extends JsonSerializer[RandomNumberGenerator] {

  override def serialize(value: RandomNumberGenerator, gen: JsonGenerator, serializers: SerializerProvider): Unit = {
    val stream: ByteArrayOutputStream = new ByteArrayOutputStream()
    val oos = new ObjectOutputStream(stream)
    oos.writeObject(value.randomData)
    oos.close()

    gen.writeStartObject()
    gen.writeNumberField("seed", value.seed)
    gen.writeStringField("state", new String(Base64.getEncoder.encode(stream.toByteArray)))
    gen.writeEndObject()
  }
}

class RandomNumberGeneratorDeserializer extends JsonDeserializer[RandomNumberGenerator] {

  override def deserialize(p: JsonParser, ctxt: DeserializationContext): RandomNumberGenerator = {
    val node: JsonNode = p.getCodec.readTree(p)
    val seed: Option[Long] = Option(node.get("seed")).map(_.asLong(System.currentTimeMillis()))
    val state: Option[String] = Option(node.get("state")).map(_.asText())

    if(state.nonEmpty) {
      val bytes = Base64.getDecoder.decode(state.get.getBytes())
      val ois = new ObjectInputStream(new ByteArrayInputStream(bytes))
      val rdg = ois.readObject.asInstanceOf[RandomDataGenerator]
      ois.close()
      new RandomNumberGenerator(rdg)
    } else {
      new RandomNumberGenerator(seed.get)
    }
  }
}
