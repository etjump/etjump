# ETJump

ETJump is a Wolfenstein: Enemy Territory trickjump modification.
 
## Required environment variables
* `ETROOT` Path to the ET installation directory.
* `GOOGLE_TEST_ROOT` Path to the Google Test framework installation directory.
* `BOOST_ROOT`  Path to the Boost library installation directory.

## Compiling 

ETJump is compiled on Windows on MSVC toolset v140. Visual C++ Redistributable 
for Visual Studio 2015 has to be installed in order to run the binaries on 
Windows. Redistributable can be downloaded from 
`https://www.microsoft.com/en-us/download/details.aspx?id=48145`. Linux binaries 
are compiled on Ubuntu 14.04. 

ETJump currently depends on 3 libraries that must be installed before ETJump can 
be compiled: SQLite3, Boost and Google Test. 

* SQLite3 can be installed on Ubuntu by running the command `sudo apt-get 
install libsqlite3-dev`. On Windows it works out of the box. Once we clean up 
the CMakeLists.txt, it should work out of the box on Ubuntu as well. 
* Boost version 1.60.0 can be downloaded from 
`http://www.boost.org/users/history/version_1_60_0.html`. Only header files are 
used. `BOOST_ROOT` environment variable should point to the Boost directory 
(e.g. `/Path/To/Boost/boost_1_60_0`). 
* Google Test can be downloaded from `https://github.com/google/googletest`. It 
must be compiled and the `gtest.lib` library must be moved to the 
googletest-release-*.*.*/lib directory. `GOOGLE_TEST_ROOT` environment variable 
should point to the googletest-release directory. 

## Building pk3 and debugging on Windows

ETJump requires 7zip `http://www.7-zip.org/` for the installation script.

Visual Studio project has a post-build event that will execute the 
build/install.bat to create the pk3 using 7zip.exe. Built pk3 will then be 
copied to $(ETROOT)/etjump directory. Starting the Debug mode will execute 
$(ETROOT)/et.exe and attach to it. 

## Unit testing

ETJump uses Google Test framework for unit testing. Some newer parts of the mod 
have unit tests. New features should have unit tests. More info on 
how to write and run tests can be found at 
`https://github.com/google/googletest`. 

