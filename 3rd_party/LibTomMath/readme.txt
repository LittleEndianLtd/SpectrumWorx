================================================================================

-----------------------------
LibTomMath LEB documentation.
-----------------------------

  Copyright © 2010.-2011. Little Endian Ltd. All rights reserved.

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

    * Download the desired LibTomMath distribution:
        https://github.com/libtom/libtommath.
    * Build the library (see below).
    * Copy the "Platform"\Debug\*.lib and "Platform"\Release\*.lib 
      (and optionaly .PDB files) as well as all *.h header files into an 
      appropriately versioned subfolder in your LEB 3rdParty libraries folder. 
      For example: 
      Header go to:
      P:/3rdParty/LibTomMath/0.42.
      Binaries for Windows 32-bit version go to:
      P:/3rdParty/LibTomMath/0.42/Win32.
      

--------------------------------------------------------------------------------

---------------------
(2.) Build procedure.
---------------------

    * See the (LE) documentation for the specific version, if any.
    * If the folder for the desired version contains a "Patches" folder, copy
      its contents (overwriting) into the installation folder of your desired
      version. 
    * Run the desired platform specific build(s) in the build folder.
        * Makefile build specific:
            * manually create the "debug" and "release" directories (names are
              case sensitive)
            * to build the release variant run "make install"
            * to build the debug variant run "make install DEBUG=1"
            * run "make clean" between builds of different variants.

--------------------------------------------------------------------------------
