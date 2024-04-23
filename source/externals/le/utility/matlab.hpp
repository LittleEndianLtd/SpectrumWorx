////////////////////////////////////////////////////////////////////////////////
///
/// \file matlab.hpp
/// ----------------
///
/// Utility wrappers for the Matlab C interop functionality.
///
/// Copyright (c) 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef matlab_hpp__CEE2A678_C3C2_4322_A84D_1F9E4E8DB580
#define matlab_hpp__CEE2A678_C3C2_4322_A84D_1F9E4E8DB580
#pragma once
#if LE_UTILITY_MATLAB_INTEROP
//------------------------------------------------------------------------------
#include "boost/range/iterator_range_core.hpp"
//------------------------------------------------------------------------------
typedef struct mxArray_tag mxArray;
typedef struct engine      Engine ;
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------
namespace Matlab
{
//------------------------------------------------------------------------------

class Engine;

class Array
{
public:
     Array();
    ~Array();

    void resize( unsigned int size );

    float * get() const;

private: friend class Engine;
    mxArray * const pArray_;
}; // class Array


class Engine
{
public:
     Engine();
    ~Engine();

    void setVariable( char const * name, Array                                const & );
    void setVariable( char const * name, boost::iterator_range<float const *> const & );

    void execute( char const * commandString ) const;

    static Engine & singleton();

private:
    ::Engine * const pEngine_;
}; // class Engine

//------------------------------------------------------------------------------
} // namespace Matlab
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // LE_UTILITY_MATLAB_INTEROP
#endif // matlab_hpp
