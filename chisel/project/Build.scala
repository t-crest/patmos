// A build Scala file for getting chisel and building the project itself.
// Martin is trying to get to the bare minimum for a Hello World

import sbt._
import Keys._

object BuildSettings {

  def apply(projectdir: String) = {
    Defaults.defaultSettings ++ Seq (
      organization := "my organization",
      version := "a version 0.1 number",
      // SBT starts with the Scala version it was build, which is 2.9.1
      // However, Chisel download assumes 2.9.2, therefore a second Scala
      // is downloaded and built. Maybe using Scala 2.9.2 for sbt would be better.
      scalaVersion := "2.9.2",
      // default source directory would be something different
      scalaSource in Compile := Path.absolute(file(projectdir + "/src")),
      libraryDependencies += "edu.berkeley.cs" %% "chisel" % "1.0.7"
//      libraryDependencies += "edu.berkeley.cs" %% "chisel" % "1.0"
    )
  }
}

object ChiselBuild extends Build {
   import BuildSettings._
   lazy val proj1 = Project(id="id1",  // a reference for setting the project
                            // with sbt (project id1)
                            base=file("hellodir"), // folder before target/
                            settings = BuildSettings(".")) // used for the project dir
                            // we build in the root 
   // if you would like to define several projects you select them in sbt with "project test2"
   //lazy val proj2 = Project(id="test2", base=file("abc"), settings = BuildSettings(".."))
}
