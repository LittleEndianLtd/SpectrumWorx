================================================================================

----------------------------------------
SpectrumWorx project main documentation.
----------------------------------------

  Copyright © 2009 - 2016. Little Endian Ltd. All rights reserved.

================================================================================

---------
Contents:
---------

    (1.) About the project.
    (2.) Basic project architecture.
    (3.) Miscellaneous notes.
    (4.) Development environment.
    (5.) Release procedure.

--------------------------------------------------------------------------------

-----------------------
(1.) About the project.
-----------------------

    * An audio plugin (AU/FMOD/VST/Unity) wrapping numerous different DSP
      algorithms/audio effects.
    * Original VST version created by Alexey Menishkov, delaydots.com.

--------------------------------------------------------------------------------

--------------------------------
(2.) Basic project architecture.
--------------------------------

    * The general design is currently documented in the (outdated)
      "spectrumworx_DSPdesigndocument.doc" document created by Alex located on
      our Trac page.
    * In addition to the architecture and components described in the SW SDK
      documentation, the SW plugin project additionally contains the following
      components:
        - GUI (individual effect/module GUIs are automatically generated through
          compiletime introspection of the their DSP specification, i.e. their
          parameters)
        - plugin protocol abstractions
        - licensing/copy protection (partially based on the Licenser SDK,
          enabled if LE_SW_AUTHORISATION_REQUIRED is defined)
        - 'external audio' (an old precursor to the separate AudioIO library)

--------------------------------------------------------------------------------

-------------------------
(3.) Miscellaneous notes.
-------------------------

    * Various random information from chats with Alex.
        * "initially, SpectrumWorx used to be pure modular with wires, etc"
          "but its not a "casual""
          "like reaktor made - its too complext for users thats why they have a
          Kore now"
          "kore is just a presets manager with best presets. this is good idea,
          i thought something like this for SpectrumWorx"
          "the trend is obvious - reaktor guys get rid from it for most
          musicans, but keep reaktor available."
        * "actually its not nessesary nowadays because host handle midi and
          parameters mapping"
          "ie you can use host to assign parameters to midi but not all hosts
          support it but most."
          "ie not sure about current stage""
          "i can control knobs by direct midi stream but there is no learn
          function for example?"
          "every modern plugin has midilearn and autio assign so, this is
          absent".

--------------------------------------------------------------------------------

-----------------------------
(4.) Development environment.
-----------------------------

    * Standard development environment (@leb/readme.txt).
    * Additional tools:
        * Windows only:
            * WiX v3.10.2 (http://wixtoolset.org/releases).
        * Mac OS X only:
            * PackageMaker (no longer shipped with Xcode, find the latest
              version @ https://developer.apple.com/downloads under "Auxiliary
              tools for Xcode").
    * Additional items used by the SW SDK project.
    * JUCE (internal patched Git snapshot).

--------------------------------------------------------------------------------

-----------------------
(5.) Release procedure.
-----------------------

    * Standard release procedure (@leb/readme.txt).
    * Additionally (before building the final package):
        * Make sure the version information (number and description) in the
          configuration files (for the main project as well as the keygen
          projects) matches the version you are trying to release.
        * Update the product's "What's new" documentation section with the
          changes for this version.
        * Generate the PDF version of the user's manual. Name it
          "User's Guide.pdf" and place it in the
          installer/ProgramFolder/Documents folder.
        * Copy/place the appropriate licence in the
          installer/ProgramFolder/Licences folder and name it "EULA.txt"
    * Windows specific:
        * The PACKAGE build step will currently fail (due to incomplete WiX
          support in CMake) but is required nonetheless.
        * After the PACKAGE build step fails, build the Release version of the
          PackageLECustom target.
        * To troubleshoot the Windows installer you can run it with logging
          enabled: msiexec /i <installer name>.msi /l*v <log name>.log
        * The installer/resources/Windows/*.wxs* were first generated by the
          builtin CMake WiX generator and then adapted. This unfortunately means
          that the list of files included in the installer is not automatically
          updated, rather the "files.wxs.in" and "features.wxs" WiX sources have
          to be manually updated.
        * WiX support in CMake has since improved: check whether this procedure
          can now be simplified.

--------------------------------------------------------------------------------
