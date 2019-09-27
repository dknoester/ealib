# Installing EALib

**These instructions are current as of 9/26/2019**

## Max OS X

### Prerequisites

1. These instructions are for XCode 11.0, Boost 1.71.0, and have been verified on Max OS X 10.14.6.  Other versions may work, YMMV.
1. These instructions install Boost to /usr/local.  If you don't install Boost there, you'll have trouble with XCode.  
1. Make sure that /usr/local/bin is on your path.
1. Note that these instructions do not install all of Boost; some libraries are left out because we don't need them.

### Step 1: XCode

Install XCode from the AppStore.

Verify that the command-line tools are installed:
```bash
   <terminal>% xcode-select --install
   xcode-select: error: command line tools are already installed, use "Software Update" to install updates
```

Verify that clang is installed:
```bash
<terminal>% clang --version
Apple clang version 11.0.0 (clang-1100.0.20.17)
Target: x86_64-apple-darwin18.7.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
<terminal>:ealib%
```

### Step 2: Boost

Download [Boost 1.71.0](https://www.boost.org/users/download) to a temporary directory (let's call it "tmp"):
```bash
    <terminal>:tmp% tar -xzf boost_1_71_0.tar.gz
    <terminal>:tmp% cd boost_1_71_0
```

Build b2 (which is Boost's equivalent of "autoconf+make"):
```bash
<terminal>:boost_1_71_0% cd tools/build
<terminal>:build% ./bootstrap.sh --with-toolset=clang
<terminal>:build% sudo ./b2 install --toolset=clang
```

Copy boost.jam:
```bash
<terminal>:build% sudo cp src/contrib/boost.jam /usr/local/share/boost-build/src/tools
```

Export BOOST_BUILD_PATH to your environment.  Assuming you use bash, put this in your \~/.bashrc file:
```bash
export BOOST_BUILD_PATH=/usr/local/share/boost-build
```

Now you're ready to build Boost.  Go up to the boost_1_71_0 directory, and:
```bash
<terminal>:boost_1_71_0% ./bootstrap.sh --with-toolset=clang --with-libraries=filesystem,iostreams,program_options,regex,serialization,system,test,timer --without-icu
<terminal>:boost_1_71_0% ./b2
```

**STOP.**  Look at the "Performing configuration checks" output (scroll back, it was the first thing printed).  Make sure that zlib is "yes."  If not, go install it (e.g., from HomeBrew).  Delete the bin.v2 directory and go back to the bootstrap step above.  Once zlib is "yes":
```bash
<terminal>:boost_1_71_0% sudo ./b2 install
```

Copy the below into ```/usr/local/share/boost-build/site-config.jam``` (you'll need to sudo):
```
    using clang ;
    import boost ;
    boost.use-project ;
    project site-config ;
    lib z ;
```

That's it.  You should have a bunch of "libboost"-prefixed files in ```/usr/local/lib``` now, and ```/usr/local/include/boost``` should be present and populated.

## Step 3: Install EALib

**IMPORTANT: Open a new terminal for Steps 3 & 4 to pick up environment changes.**

Clone EALib to a new directory, and verify it builds:

```bash
<terminal>:src% git clone https://github.com/dknoester/ealib.git ealib
<terminal>:src% cd ealib
<terminal>:ealib% b2
...patience...
...found 457 targets...
...updating 47 targets...
clang-darwin.compile.c++ bin/clang-darwin-11.0/debug/link-static/examples/all_ones.o
clang-darwin.compile.c++ bin/clang-darwin-11.0/debug/link-static/examples/lod.o
clang-darwin.compile.c++ bin/clang-darwin-11.0/debug/link-static/examples/pole_balancing.o
clang-darwin.compile.c++ bin/clang-darwin-11.0/debug/link-static/examples/logic9.o
...
*** No errors detected
clang-darwin.compile.c++ libea/bin/clang-darwin-11.0/debug/test/test_neurodevelopment.o
clang-darwin.compile.c++ libea/bin/clang-darwin-11.0/debug/test/test_neuroevolution.o
clang-darwin.link libea/bin/clang-darwin-11.0/debug/neural_network
testing.unit-test libea/bin/clang-darwin-11.0/debug/neural_network.passed
Running 4 test cases...

*** No errors detected
clang-darwin.compile.c++ libea/bin/clang-darwin-11.0/debug/test/test_markov_network_ea.o
clang-darwin.link libea/bin/clang-darwin-11.0/debug/markov_network
testing.unit-test libea/bin/clang-darwin-11.0/debug/markov_network.passed
Running 4 test cases...

*** No errors detected
...updated 47 targets...
<terminal>:ealib%
```

There will be a couple warnings.  Happy to accept pull requests to fix them.

Now let's verify that XCode works.  First, set the Mac OS deployment target:

1. Open ealib/ealib.xcodeproj in XCode.
1. In project navigator (folder icon, upper left), click the ealib project (top of list, in blue).
1. In the target-viewer pane (middle) click the ealib project (top), then the "Info" tab, make sure that the deployment target is set to the version of Mac OS you're using.

Finally, let's verify that it builds:
1. Highlight the "all" target (upper left drop-down, right of the "stop" button)
1. Command-B (Product -> Build)

There should be lots of warnings, but no errors.    
    
## (Bonus) Step 4: Install Avida4

**After** you've installed Boost & EALib, clone the Avida4 project into the directory along-side EALib:

```bash
<terminal>:src% git clone https://github.com/dknoester/avida4.git avida4
<terminal>:src% ls
```bash
avida4 ealib
<terminal>:src%
```

Build Avida4:
```bash
<terminal>:src% cd avida4
<terminal>:avida4% b2
...found 192 targets...
...updating 10 targets...
clang-darwin.compile.c++ ../ealib/libea/bin/clang-darwin-11.0/debug/link-static/src/expansion.o
clang-darwin.compile.c++ ../ealib/libea/bin/clang-darwin-11.0/debug/link-static/src/main.o
clang-darwin.compile.c++ ../ealib/libea/bin/clang-darwin-11.0/debug/link-static/src/cmdline_interface.o
clang-darwin.archive ../ealib/libea/bin/clang-darwin-11.0/debug/link-static/libea_runner.a
clang-darwin.archive ../ealib/libea/bin/clang-darwin-11.0/debug/link-static/libea_cmdline.a
clang-darwin.compile.c++ bin/clang-darwin-11.0/debug/link-static/src/logic9.o
clang-darwin.link bin/clang-darwin-11.0/debug/link-static/avida-logic9
common.copy /Users/toaster/bin/avida-logic9
...updated 10 targets...
<terminal>:ealib%
```

And test:
```bash
<terminal>:avida4% ./bin/clang-darwin-11.0/debug/link-static/avida-logic9 -c etc/logic9.cfg --verbose

Active configuration options:
    config=etc/logic9.cfg
    ea.environment.x=60
    ea.environment.y=60
    ea.mutation.deletion.p=0.05
    ea.mutation.insertion.p=0.05
    ea.mutation.site.p=0.0075
    ea.population.size=3600
    ea.representation.size=100
    ea.run.checkpoint_name=checkpoint.xml
    ea.run.epochs=1
    ea.run.updates=100
    ea.scheduler.resource_slice=30
    ea.scheduler.time_slice=30
    ea.statistics.recording.period=10
    verbose=

update instantaneous_t average_t memory_usage
0 0.0004 0.0004 3.8828
1 0.0000 0.0002 3.8828
2 0.0000 0.0001 3.8828
3 0.0001 0.0001 3.8828
...
97 0.0008 0.0003 4.2695
98 0.0008 0.0003 4.2695
99 0.0007 0.0003 4.2695
<terminal>:avida4%
```

For XCode, **first close the EALib project if you have it open!**  Then, open avida4/avida4.**xcworkspace** -- **NOT** the project!  The workspace brings in the EALib project as a dependency, which is also why it's important that avida4 and ealib repos live in the same directory as each other.

You'll need to set the deployment target for Avida4 now (project navigator -> avida4 -> avida4 -> Info tab), and then build by selecting the "all (avida4 project)" project.  Again, there should be warnings but no errors.

That's it!

## Previous (2015) instructions for HPCC

Note: The below is severely outdated, and in need of updating.  Happy to accept pull requests.  A better way to do this is probably to containerize your project?  Not sure, but see here: https://wiki.hpcc.msu.edu/display/ITH/Singularity.

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


