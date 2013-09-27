name := "Patmos"

scalaVersion := "2.9.2"

resolvers ++= Seq(
  "scct-github-repository" at "http://mtkopone.github.com/scct/maven-repo"
)

scalaSource in Compile := new File("src")

libraryDependencies += "edu.berkeley.cs" %% "chisel" % "1.0.8"
