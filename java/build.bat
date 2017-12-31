
mkdir tqrget
mkdir target\out
del /s /q target\out


javac -encoding utf8 -d target\out @files
jar cvf target\tqapi.jar -C target\out .

