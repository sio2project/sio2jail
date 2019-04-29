sio2jail
========


building
--------

You need a CMake, a C/C++ compiler with multilib support and python2. Any
external libraries sio2jail use (see below) can be either installed
system-wide, or downloaded and built during the process. To build sio2jail and
install files to ~/local directory run:

    mkdir build && cd build
    
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/local ..
    make && make install

Our sio2jail uses some external libraries and programs:
  * libcap
  * libseccomp (>= 2.3.0)
  * libtclap
  * scdoc (for generating man pages)

some of which you can install (e.g. on Debian) with:

    apt-get install libcap-dev libtclap-dev libseccomp-dev

By default sio2jail searches for this libraries in system paths and in case they
aren't found their sources are downloaded and libraries are built in working
directory. You can tune this behaviour with cmake options:

    -D<DEPENDENCY>_PREFIX=<PATH>
    -D<DEPENDENCY>_BUILD_OWN=YES|NO

where DEPENDENCY is one of
  * LIBCAP
  * LIBSECCOMP
  * LIBTCLAP
  * SCDOC

You can also control wheather to generate man pages with option (YES by default):

    -DWITH_DOCS=YES|NO

and wheather to install boxes scripts (NO by default):

    -DWITH_BOXES=YES|NO

To control wheather sio2jail binary is statically or dinamically linked use
option (STATIC by default):

    -DLINK=STATIC|DYNAMIC

There is also a possibility to control wheather output binary should run on other
architecture than the defualt one (or force given architecture):

    -DARCH=i386|x86_64

Note, that when using ARCH other than build host architecture it may be necessary
(depending on libraries installation) to build sio2jail with custom libseccomp (more
precisly with flag -DLIBSECCOMP\_BUILD\_OWN=YES).

For example, to skip man pages, use libtclap from /opt/tclap directory and
ignore system libseccomp run:

    cmake -DWITH_DOCS=NO -DLIBTCLAP_PREFIX=/opt/tclap -DLIBSECCOMP_BUILD_OWN=YES ..

running
-------

You may need to run

    sysctl -w kernel.perf_event_paranoid=-1
    sysctl -w kernel.unprivileged_userns_clone=1

or to enable these options in your /etc/sysctl.conf in order
to run sio2jail.

running tests
-------------

To run test suit use 'check' target, e.g in build directory run:

    make check
