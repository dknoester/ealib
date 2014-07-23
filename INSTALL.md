========
MSU HPCC (QUICK WAY):

Put this:
    export BOOST_BUILD_PATH=/mnt/home/dk/share/boost-build

... in your ~/.bashrc file (or other shell startup script if you prefer).  That should do it.


========
MSU HPCC (ROLL YOUR OWN, AT YOUR OWN RISK, INSTALLS IN YOUR HOME DIR): 

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

Build boost:
    cd ../../..
    ./bootstrap.sh --prefix=${HOME}
    ./b2
    ./b2 install

Set the boost build root; add this:
    export BOOST_BUILD_PATH=$HOME/share/boost-build
... to ~/.bashrc
