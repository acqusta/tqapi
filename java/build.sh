#!/bin/sh

if [ -e target/out ]; then
   rm -rf target/out
fi

mkdir -p target/out

javac -d target/out @files
jar cvf target/tqapi.jar -C target/out .

