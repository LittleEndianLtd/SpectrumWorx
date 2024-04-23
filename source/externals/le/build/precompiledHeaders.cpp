////////////////////////////////////////////////////////////////////////////////
///
/// precompiledHeaders.cpp
/// ----------------------
///
///   Required because of the way MSVC implementes precompiled headers. To
/// enable the usage of LE precompiled headers in a MSVC project you have to:
///   - enable precompiled headers in the project properties and set it to use
///     the "precompiledHeader.hpp" file for precompiled header file creation
///   - either manually either automatically (through forced inclusion) include
///     the "precompiledHeader.hpp" file in all translation units
///   - add the "precompiledHeader.cpp" file project and edit its specific
///     compile options and under "Precompiled Header" select the "/Yc" option
///     (for all configurations), all other translation units should use the
///     project default which should be set to the "/Yu" option
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "precompiledHeaders.hpp"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------