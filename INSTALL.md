** These instructions are current as of 7/22/2014 **

There are Three different sets of instructions for installing Boost here: HPCC (quick way), HPCC (self-built), and OS X (xcode version 5.1.1, OS X 10.9.4).

**After** you've installed Boost, get a copy of the latest EALib and Avida4 sample project:

    git clone https://github.com/dknoester/ealib.git ealib
    git clone https://github.com/dknoester/avida4.git avida4

To make sure everything's working:

    cd ealib
    bjam
    cd ../avida4
    bjam

A sample run (from avida4/):

    ./bin/clang-darwin-4.2.1/debug/link-static/avida-logic9 -c ./etc/logic9.cfg --verbose


========
MSU HPCC (QUICK WAY):
========

Put these:
    export BOOST_BUILD_PATH=/mnt/home/dk/share/boost-build
    export PATH=$PATH:/mnt/home/dk/bin

... in your ~/.bashrc (or other shell startup script).  That should do it.


========
MSU HPCC (ROLL YOUR OWN, AT YOUR OWN RISK, INSTALLS IN YOUR HOME DIR): 
========

Make sure that your path includes ${HOME}/bin, e.g., put this:
    export PATH=$PATH:${HOME}/bin
... in your ~/.bashrc (or other shell startup script).

Download boost (current rls == 1.55):
    wget http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz
    tar -xzf boost_1_55_0.tar.gz
    cd boost_1_55_0

Build bjam:
    cd tools/build/v2
    ./bootstrap.sh
    ./b2 install --prefix=${HOME}

Site config:
    vi ~/share/boost-build/site-config.jam
>>>
using gcc : : : <compileflags>-ftemplate-depth-255 ;
import os ;
local HOME = [ os.environ HOME ] ;
local INC = $(HOME)/include ;
local LIB = $(HOME)/lib ;
local SYS = /usr/lib64 ;

import boost ;
using boost : 1.55 :
    <include>$(INC)
    <library>$(LIB)
    <layout>system
    ;
boost.use-project 1.55 ;

project site-config : requirements <include>$(INC) ;
lib z : : <link>shared <name>z <search>$(SYS) ;
>>>

Copy boost.jam:
    cp contrib/boost.jam ~/share/boost-build/

Set the boost build root; add this:
    export BOOST_BUILD_PATH=$HOME/share/boost-build
... to your ~/.bashrc (or other shell startup script).

Build boost:
    (back in boost_1_55_0/)
    ./bootstrap.sh --prefix=${HOME}
    ./b2
    ./b2 install

That should be it...


========
OS X
========

These instructions are for XCode 11.0.  They install Boost to /usr/local.  Make sure that /usr/local/bin is on your path.  If you don't install Boost there, you'll have trouble with XCode.  Also, these instructions do not install all of Boost; some libraries are left out because we don't need them.

Install XCode from the AppStore.

Verify that the command-line tools are installed:
    xcode-select --install

Download Boost (this has been verified w/ Boost 1.71; use other versions at your own risk):
    tar -xzf boost_1_71_0.tar.gz
    cd boost_1_71_0

Build bjam:
    cd tools/build
    ./bootstrap.sh --with-toolset=clang
    sudo ./b2 install --toolset=clang

Copy boost.jam:
    sudo cp src/contrib/boost.jam /usr/local/share/boost-build/src/tools

Put this in your ~/.bashrc:
    export BOOST_BUILD_PATH=/usr/local/share/boost-build

Build boost:
    (back in boost_<<DIRNAME>>)
    ./bootstrap.sh --with-toolset=clang --with-libraries=filesystem,iostreams,program_options,regex,serialization,system,test,timer --without-icu
    ./b2

STOP.  Look at the "Performing configuration checks" output (scroll back, it was the first thing printed).  Make sure that zlib is "yes."  If not, go install it (e.g., from MacPorts or HomeBrew).  Delete the bin.v2 directory and go back to the bootstrap step above.  Then...

    sudo ./b2 install

Site config:
    Copy the below into /usr/local/share/boost-build/site-config.jam (you'll need to sudo):

    using clang ;
    import boost ;
    boost.use-project ;
    project site-config ;
    lib z ;

Ensure that the Mac OS deployment target is correct:
    Open ealib.xcodeproj
    In project viewer, click ealib project (top).
    Under ealib project, info tab, make sure that the deployment target is set to the version of Mac OS you're using.
    
    
