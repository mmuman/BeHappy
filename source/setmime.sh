#!/bin/sh

settype -t application/x-scode-DPkg BHPackage
settype -t text/x-source-code *.h *.cpp Add-ons/*.cpp Add-ons/*.h
settype -t text/html BH BH.*
settype -t image/jpeg BH.jpeg
settype -t application/x-be-resource *.rsrc
settype -t application/x-mw-project *.proj Add-ons/*.proj
settype -t text/html Doc/en/*.html Doc/fr/*.html Doc/hu/*.html Doc/nl/*.html
settype -t image/gif Doc/en/*.gif Doc/fr/*.gif Doc/hu/*.gif Doc/nl/*.gif
settype -t image/jpeg Doc/en/*.jpeg Doc/fr/*.jpeg Doc/hu/*.jpeg Doc/nl/*.jpeg
