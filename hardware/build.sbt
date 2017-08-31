name := "Patmos"

scalaVersion := "2.11.7"

scalacOptions ++= Seq("-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

libraryDependencies <+= scalaVersion("org.scala-lang" % "scala-compiler" % _)

scalaSource in Compile <<= baseDirectory(_ / "src")

libraryDependencies += "edu.berkeley.cs" %% "chisel" % "2.2.33"
