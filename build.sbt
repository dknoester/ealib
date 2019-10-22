name := "scalatest"

version := "0.1"

scalaVersion := "2.13.1"

resolvers += "mvnrepository" at "http://mvnrepository.com/artifact/"

libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.8" % "test",
  // https://mvnrepository.com/artifact/org.apache.commons/commons-math3
  "org.apache.commons" % "commons-math3" % "3.6.1",
  // https://mvnrepository.com/artifact/com.fasterxml.jackson.module/jackson-module-scala
  "com.fasterxml.jackson.module" %% "jackson-module-scala" % "2.10.0",
  // https://mvnrepository.com/artifact/org.rogach/scallop
  "org.rogach" %% "scallop" % "3.3.1"
)