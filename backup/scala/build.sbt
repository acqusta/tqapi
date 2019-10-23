organization    := "tquant"

name            := "tqc-api-scala"

version         := "1.0-SNAPSHOT"

//scalaVersion    := "2.11.8"

javacOptions   ++= Seq("-encoding", "UTF-8")

libraryDependencies += "org.zeromq"                    % "jeromq"               % "0.4.2"

val jacksonVersion = "2.9.1"
libraryDependencies += "com.fasterxml.jackson.core"    % "jackson-core"         % jacksonVersion
libraryDependencies += "com.fasterxml.jackson.core"    % "jackson-annotations"  % jacksonVersion
libraryDependencies += "com.fasterxml.jackson.module" %% "jackson-module-scala" % jacksonVersion

libraryDependencies += "org.msgpack" % "jackson-dataformat-msgpack" % "0.8.12"
