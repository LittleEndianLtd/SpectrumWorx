================================================================================

------------------------------
LibTomCrypt LEB documentation.
------------------------------

  Copyright © 2010. Little Endian Ltd. All rights reserved.

================================================================================

---------
Contents:
---------

    (1.) Build procedure.
    (2.) Installation.

--------------------------------------------------------------------------------

------------------
(1.) Installation.
------------------

    * Download the desired LibTomCrypt distribution:
        https://github.com/libtom/libtomcrypt.
    * Build the library (see below).
    * Copy the "Platform"\Debug\*.lib and "Platform"\Release\*.lib 
      (and optionaly .PDB files) as well as the complete "src/headers" folder 
      into an appropriately versioned subfolder in your LEB 3rdParty libraries 
      folder. For example:
      P:/3rdParty/LibTomCrypt/1.17. 
      In this example headers go to:
      P:/3rdParty/LibTomCrypt/1.17/src/headers.
      and binaries for Windows 32-bit go to:
      P:/3rdParty/LibTomCrypt/1.17/Win32.

--------------------------------------------------------------------------------

---------------------
(2.) Build procedure.
---------------------

    * See the (LE) documentation for the specific version, if any.
    * If the folder for the desired version contains a "Patches" folder, copy
      its contents (overwriting) into the installation folder of your desired
      version. 
    * Run the desired platform specific build(s) in the build folder.
        * Makefile build specific: to build the release variant simple run
        "make", to build the debug variant run "make DEBUG=1". Run "make clean"
        between builds of different variants. You must also manually create the
        "debug" and "release" directories before building.

--------------------------------------------------------------------------------
