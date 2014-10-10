Blackbox
========

The Blackbox plugins are a set of custom plugins designed to extend the ChucK programming language. 


Setup
=====

In order to use these plugins, they must be compiled with Chuck. Blackbox is designed (in an admittedly unconventional way) to be as decoupled from the chuck sourcecode as possible. 

Blackbox needs the following in order to build:
ChucK sourcecode
Csound 6 (preferably compiled from source)
libsndfile (required by Csound and Chuck)

0. Compile Csound. Instructions (and dependencies) for compilation can be found [here](https://github.com/csound/csound/blob/develop/BUILD.md).
1. Download the most reason of ChucK (in this case, it was 1.3.4.0) and extract it
2. Place the blackbox directory in src/
3. In the blackbox directory, there is a file called blackbox.patch. Place it in the top level of the Chuck directory
4. Patch the source code. This will slightly modify some of chuck's source code so it knows to look in the blackbox directory. This can be done with the command "patch -p1 < blackbox.patch"
5. Go into the src/ directory and run make PLATFORM, where platform is the operating system you are building for. Run "make" to see a list of the possible options.
6. run "sudo make install" to install. 




