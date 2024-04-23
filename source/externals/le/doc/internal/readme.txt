================================================================================

---------------------------------------
Little Endian Build main documentation.
---------------------------------------

  Copyright © 2009 - 2016. Little Endian Ltd. All rights reserved.

================================================================================

---------
Contents:
---------

    (1.) About the project.
    (2.) Base requirements for all projects.
    (3.) Standard development environment
    (4.) Standard release procedure.

--------------------------------------------------------------------------------

-----------------------
(1.) About the project.
-----------------------

    * Provides base functionality and documentation shared and/or required by
      build procedures of other projects.
    * Provides (custom) build procedures and documentation for various 3rd party
      libraries.

--------------------------------------------------------------------------------

----------------------------------------
(2.) Base requirements for all projects.
----------------------------------------

    * All projects use the CMake build system (https://cmake.org). The standard/
      common procedure for setting up project build with CMake is as follows:
        * Open CMake.
        * Set the source code folder to the "source" folder (the one containing
          the CMakeLists.txt file).
        * Set the binary folder according to your preference (this is where
          the platform & IDE specific files will be placed, like Visual Studio
          solution&project files).
        * Press "Configure".
        * Choose the desired generator (and toolchain file if crosscompiling).
        * Press "Configure".
        * Press "Generate".
        * CMake can (but does not have to) be closed at this point.
        * Open the generated native project using the appropriate IDE.

    * All C and C++ projects must first (force) include the root ODR and ABI
      header provided by this project.
        * They can optionaly configure it using the LE_CHECKED_BUILD macro (see
          the header for more details).

    * All 3rd party libraries should be placed in a single folder and an
      environment variable, named "LEB_3rdParty_root", that points to that
      folder should be created. The path must use forward slashes and not
      include the trailing slash.
    * Some projects may require an additional environment variable,
      named "LEB_root", that points to the root of the LEB folder structure
      (such that ${LEB_root}/doc/trunk/internal/readme.txt points to this
      document).
    * Mac OS X specific:
        * To make sure that environment variables are properly visible to all
          applications, all environment variables should be set in the
          /etc/launchd.conf file. By default this file does not exist and it
          needs to be created as a plain text file. Individual variables are
          added each on a separate line with the following syntax:
          setenv <name of variable> <variable value>.
          (IMPORTANT: Apple doesn't really like environment variables and has
          the habit of breaking this process in almost every major release so
          always make sure that the method used for setting the env.vars still
          works after an OS update.)

--------------------------------------------------------------------------------

--------------------------------------
(3.) Standard development environment.
--------------------------------------

    * Tools:
        * CMake 3.5.1
        * Doxygen 1.8.11
        * supported platforms:
            * Windows - Visual Studio (2013 Update 5)
            * OS X    - Xcode (7.3)
            * iOS     - consult iOS.txt
            * Android - consult android.txt

        * Avast antivirus notice: disable autosandboxing of files with a low
          prevalence/reputation (or at least set AutoSandbox mode to Ask) to
          prevent strange or silent failures of custom tools used by the build
          configuration process.

    * External libraries and tools.
        * Boost 1.61.0 (autoinstalled)
            - version specified with the "boostVersion" variable in
              buildOptions.cmake
        * All third party libraries are assumed to be built and installed
          as specified by LEB documentation.


--------------------------------------------------------------------------------

--------------------------------------------------------------------------------

--------------------------------
(4.) Standard release procedure.
--------------------------------

    1. Make sure you have a clean SVN source tree of the project and all 3rd
       party libraries.

    2. Update the product's "Release notes" documentation section with the
       changes for this version.

    3. Build the installation package.
        * If building for a single configuration (platform, effect set, etc.):
            * (re)create the build files with the LE_CREATE_PACKAGE_TARGET CMake
              option enabled (some projects do not have this option and behave
              as if it is always enabled).
            * build the PACKAGE sub project ("release" configuration).
        * If building all/many configurations (e.g. for a final release) and if
          the project contains the batchBuild.cmake script (usually in the
          'build' or the 'source' directory), use the script to build all
          supported versions in a single go (consult the script for usage
          details).

    4. Create an appropriate SVN branch and/or SVN tag for the released version.

--------------------------------------------------------------------------------
