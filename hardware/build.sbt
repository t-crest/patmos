name := "Patmos"

scalaVersion := "2.12.17"

scalacOptions ++= Seq("-Xsource:2.11", "-unchecked", "-deprecation", "-feature", "-language:reflectiveCalls")

Compile / unmanagedSourceDirectories += baseDirectory.value / "../../soc-comm/src"



// Chisel 3.4
/*
libraryDependencies += "edu.berkeley.cs" %% "chisel3" % "3.4.0"
// the following was not needed when using iotesters...
libraryDependencies += "org.scala-lang.modules" % "scala-xml_2.12" % "1.2.0"
// libraryDependencies += "edu.berkeley.cs" %% "chisel-iotesters" % "1.5.1"
libraryDependencies += "edu.berkeley.cs" %% "chiseltest" % "0.3.0"
*/

// the following version uses the compiler plugin, but gives errors with the Argo wrapper (LEfteris version)
/* 
name := "Patmos"

scalaVersion := "2.12.12"

scalacOptions ++= Seq(
  "-deprecation",
  "-feature",
  "-unchecked",
  "-language:reflectiveCalls",
)

val chiselVersion = "3.4.4"
addCompilerPlugin("edu.berkeley.cs" %% "chisel3-plugin" % chiselVersion cross CrossVersion.full)
libraryDependencies += "edu.berkeley.cs" %% "chisel3" % chiselVersion
libraryDependencies += "edu.berkeley.cs" %% "chiseltest" % "0.3.0"
 */

/* not yet 2.13
scalaVersion := "2.13.10"

scalacOptions ++= Seq(
  "-deprecation",
  "-feature",
  "-unchecked",
  "-language:reflectiveCalls",
)
*/

 // Chisel 3.5
val chiselVersion = "3.5.5"
addCompilerPlugin("edu.berkeley.cs" %% "chisel3-plugin" % chiselVersion cross CrossVersion.full)
libraryDependencies += "edu.berkeley.cs" %% "chisel3" % chiselVersion
libraryDependencies += "edu.berkeley.cs" %% "chiseltest" % "0.5.5"

// For FIFO buffers
libraryDependencies += "edu.berkeley.cs" % "ip-contributions" % "0.5.1"
libraryDependencies += "com.fazecast" % "jSerialComm" % "[2.0.0,3.0.0)"
