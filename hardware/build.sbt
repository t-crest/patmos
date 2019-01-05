name := "Patmos"

scalaVersion := "2.11.7"

scalacOptions ++= Seq("-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

libraryDependencies += scalaVersion("org.scala-lang" % "scala-compiler" % _).value

resolvers ++= Seq(
  Resolver.sonatypeRepo("snapshots"),
  Resolver.sonatypeRepo("releases")
)

// here switch between Chisel 2 and 3

libraryDependencies += "edu.berkeley.cs" %% "chisel" % "2.2.38"

//libraryDependencies += "edu.berkeley.cs" %% "chisel3" % "3.1.2"
//libraryDependencies += "edu.berkeley.cs" %% "chisel-iotesters" % "1.2.2"
