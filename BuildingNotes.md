# Building from the Google Code #

My primary build computers are Macintoshes, so the Makefile is somewhat Mac-centric.  You will have to figure out how to change the Makefile yourself for Linux builds.

## Building on OS X ##

You need the Apple Developer Tools installed.

After checking out the code, just create an empty file, "Make.dep" in the top directory and then run 'make.'  It should just work.

## Building on Ubuntu ##

You will need 'g++' and 'tcl-dev' packages installed.

```
$ sudo apt-get install tcl8.5-dev
$ sudo apt-get install g++
```

Edit the Makefile so that it includes Make.ubuntu file.

Create an empty file, Make.dep.

If you installed another version of Tcl (say, 8.3) change the line in Make.ubuntu to read:
```
TCL_VERSION=8.3
```

## Building on Windows ##

You are on your own here, sorry.