#!/bin/sh
FILES="Add-On?Test.proj Add-On.cpp Add-On.h Add-On.proj BHAdd-ons.h HappyCommander.h HappyCommander.cpp HappyLink.h HTMLFile.h HTMLProject.h Parser HappyTest.a"

rm -R BeHappyDK
mkdir BeHappyDK

for file in $FILES
do
  cp ../$file BeHappyDK
  echo copie de $file
done

cp ReadMe BeHappyDK
echo copie de ReadMe

cp -R Doc BeHappyDK
echo copie de la doc

cp -R ../Examples BeHappyDK
rm -R BeHappyDK/Examples/CVS
rm -R BeHappyDK/Examples/*/CVS
echo copie des exemples

settype -t text/plain BeHappyDK/ReadMe
settype -t application/x-mw-project BeHappyDK/*.proj
settype -t text/x-source-code BeHappyDK/*.h BeHappyDK/*.cpp
settype -t application/x-vnd.Be.ar-archive BeHappyDK/*.a
settype -t text/html BeHappyDK/Doc/*.html BeHappyDK/Doc/DK/*.html
settype -t image/gif BeHappyDK/Doc/*.gif BeHappyDK/Doc/DK/*.gif
settype -t image/jpeg BeHappyDK/Doc/*.jpeg
settype -t text/x-source-code BeHappyDK/Examples/*/*
echo reglage des types MIME
