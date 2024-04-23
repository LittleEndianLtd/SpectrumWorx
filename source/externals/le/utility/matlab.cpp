////////////////////////////////////////////////////////////////////////////////
///
/// matlab.cpp
/// ----------
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if LE_UTILITY_MATLAB_INTEROP
//------------------------------------------------------------------------------
#include "matlab.hpp"

#include "engine.h"
#include "matrix.h"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
#pragma comment( lib, LE_UTILITY_MATLAB_INTEROP_LIB_DIR "libmx.lib"  )
#pragma comment( lib, LE_UTILITY_MATLAB_INTEROP_LIB_DIR "libmat.lib" )
#pragma comment( lib, LE_UTILITY_MATLAB_INTEROP_LIB_DIR "libeng.lib" )
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

Array:: Array() : pArray_( mxCreateNumericMatrix( 1, 0, mxSINGLE_CLASS, mxREAL ) ) { BOOST_VERIFY( pArray_ ); }
Array::~Array() { ::mxDestroyArray( pArray_ ); }

void Array::resize( unsigned int const size )
{
    ::mxSetData( pArray_, ::mxRealloc( ::mxGetData( pArray_ ), size * sizeof( float ) ) );
    ::mxSetN   ( pArray_, size );
}

float * Array::get() const { return static_cast<float *>( ::mxGetData( pArray_ ) ); }


Engine:: Engine() : pEngine_( ::engOpen( nullptr ) ) { BOOST_ASSERT( pEngine_ ); }
Engine::~Engine()                                    { BOOST_VERIFY( ::engClose( pEngine_ ) == 0 ); }

void Engine::setVariable( char const * const name, Array const & value ) { BOOST_VERIFY( ::engPutVariable( pEngine_, name, value.pArray_ ) == 0 ); }

void Engine::setVariable( char const * const name, boost::iterator_range<float const *> const & value )
{
    static Matlab::Array array;
    auto const size( static_cast<unsigned int>( value.size() ) );
    array.resize( size );
    std::memcpy( array.get(), value.begin(), size * sizeof( value.front() ) );
    setVariable( name, array );
}

void Engine::execute( char const * const commandString ) const { BOOST_VERIFY( ::engEvalString( pEngine_, commandString ) == 0 ); }

Engine & Engine::singleton() { static Engine singleton_; return singleton_; }

//------------------------------------------------------------------------------
} // namespace Matlab
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // LE_UTILITY_MATLAB_INTEROP
