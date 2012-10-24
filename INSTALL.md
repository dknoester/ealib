# EALib Installation Guide #


## Conventions ##
Text `like this` refers to something that is case & punctuation specific, for example, a path like `/usr/local`.  

When something needs to be executed in a terminal, it will look `% like this`.  Note the preceding `%`-sign.


## Prerequisites ##
- You need a working development environment.  On OS X, this is Xcode, which can be downloaded from the App Store.  On Ubuntu, this can be done by:

        % sudo apt-get install build-essential

- Decide where you are going to install the Boost libraries.  This is called the prefix.  The default prefix on Unix-like systems is `/usr/local`.  If you don't have root permissions on your system, you will need to use your home directory as the prefix.  In the rest of this document, wherever you see *prefix*, either: omit it if you are using the default, or replace it with your home directory.  For example, `% ./bootstrap --prefix=prefix` means execute `./bootstrap.sh` in a terminal if you are using the default location, or execute `./bootstrap.sh --prefix=${HOME}` in a terminal if you are using your home directory.  Note that `${HOME}` is shorthand for your home directory on Unix-like systems.  

- If you are installing to /usr/local, you'll need to have root permissions.  In the directions that follow, wherever you see sudo, if you are NOT installing to /usr/local, you should NOT include the sudo command.  E.g., if you are installing to your home directory and you see `% sudo ./b2 install`, you should execute "./b2 install" instead of "sudo ./b2 install".

- Install zlib.  This *should* be installed by default, but you might have to go get it from MacPorts (OS X) or other platform-specific package repository.  If you're not sure if it's installed, check /usr/lib, /opt/local/lib, and /usr/local/lib for libz.* (your system is not likely to have all these directories present; that's ok).
