
mkdir target
mkdir target\classes
del /s /q target\classes


javac -encoding utf8 -d target\classes @files
jar cvf target\tqapi.jar -C target\classes .

