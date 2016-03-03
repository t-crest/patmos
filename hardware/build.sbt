name := "Patmos"

scalaVersion := "2.10.2"

addSbtPlugin("com.github.scct" %% "sbt-scct" % "0.2")

scalacOptions ++= Seq("-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

libraryDependencies <+= scalaVersion("org.scala-lang" % "scala-compiler" % _)

scalaSource in Compile <<= baseDirectory(_ / "src")

libraryDependencies += "edu.berkeley.cs" %% "chisel" % "2.2.31"
