name := "Patmos"

scalaVersion := "2.12.12"

scalacOptions ++= Seq("-Xsource:2.11", "-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

// Chisel 3.4
libraryDependencies += "edu.berkeley.cs" %% "chisel-iotesters" % "1.5.1"

