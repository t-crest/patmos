name := "Patmos"

scalaVersion := "2.12.12"

scalacOptions ++= Seq("-Xsource:2.11", "-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

libraryDependencies += scalaVersion("org.scala-lang" % "scala-compiler" % _).value

// Chisel 3.4
libraryDependencies += "edu.berkeley.cs" %% "chisel-iotesters" % "1.5.1"

