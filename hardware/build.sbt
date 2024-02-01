name := "Patmos"

scalaVersion := "2.12.12"

scalacOptions ++= Seq("-Xsource:2.11", "-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

// Chisel 3.4
libraryDependencies += "edu.berkeley.cs" %% "chisel3" % "3.4.0"
// the following was not needed when using iotesters...
libraryDependencies += "org.scala-lang.modules" % "scala-xml_2.12" % "2.2.0"
// libraryDependencies += "edu.berkeley.cs" %% "chisel-iotesters" % "1.5.1"
libraryDependencies += "edu.berkeley.cs" %% "chiseltest" % "0.3.0"

