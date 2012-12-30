Chorus Suite - Playback Module
==============================

This is largely a fork of the squeezeslave project with a focus on modularisation and a minimum of bundled dependencies.  

What is the Chorus Suite?
-------------------------

Chorus is a project to provide a new player for the SqueezeBox ecosystem with a focus on desktop usability.  

Why a fork?
-----------
SqueezeSlave was forked rather than included as it was found that significant modifications were needed to build and use the library components separately to the main squeezeslave binary.  Also, some functionality has been lost in the split, and many bundled components thrown out (to keep the repository small and tidy). 

Building
--------

Check out the source code, install CMake if you haven't already and build the project in usual CMake fashion:
    
    mkdir build
    cd build
    cmake ..
    make

(on Unix, Windows will be similar)
