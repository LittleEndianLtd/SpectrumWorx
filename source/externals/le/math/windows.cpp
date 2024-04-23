////////////////////////////////////////////////////////////////////////////////
///
/// windows.cpp
/// -----------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "windows.hpp"

#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"

#include "boost/assert.hpp"

#include <algorithm>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

#pragma warning( push )
#pragma warning( disable : 4244 ) // Conversion from 'window_t' to 'float', possible loss of data.

LE_COLD
void LE_FASTCALL calculateWindow( DataRange const & window, LE::SW::Engine::Constants::Window const windowType )
{
    // http://en.wikipedia.org/wiki/Window_function
    // "On the Use of Windows for Harmonic Analysis with the Discrete Fourier
    // Transform", Frederic J. Harris 1978
    // http://web.mit.edu/xiphmont/Public/windows.pdf
    // "Spectrum and spectral density estimation by the Discrete Fourier
    // transform (DFT), including a comprehensive list of window functions and
    // some new at-top windows"
    // http://edoc.mpg.de/395068
    // http://www.katjaas.nl/FFTwindow/FFTwindow.html
    // http://www.sandv.com/downloads/0603gabe.pdf
    // http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/windows
    // http://zone.ni.com/devzone/cda/tut/p/id/4844
    // http://dafx10.iem.at/proceedings/papers/Helmrich_DAFx10_P32.pdf
    // http://www.dewresearch.com/downloads/FFTPropertiesTutorial.pdf
    // http://www.gearslutz.com/board/mastering-forum/200709-blackman-harris-hanning-hamming.html
    // http://www.cg.tuwien.ac.at/research/vis/vismed/Windows/MasteringWindows.pdf

    using namespace     Math;
    using namespace LE::Math::Constants;

    namespace Window = LE::SW::Engine::Constants;

    float       * LE_RESTRICT       pWindow( window.begin() );
    float const *             const pEnd   ( window.end  () );

    /// \note
    /// We want so called "DFT-even" windows, suited for spectral analysis.
    /// These windows need to have the same period as the DFT, so their last
    /// sample needs to be equal to their second, not first, sample. For more
    /// information see the second page (left column) of the Harris book and/or
    /// the following links:
    /// http://zone.ni.com/reference/en-XX/help/370051H-01/cvi/windows_for_spectral_analysis_versus_windows_for_coefficient_design
    /// http://cow.physics.wisc.edu/~craigm/idl/archive/msg07112.html
    /// http://en.wikipedia.org/wiki/Window_function#Window_examples.
    ///                                       (17.04.2012.) (Domagoj Saric)

    //...mrmlj...(BandGain)...BOOST_ASSERT_MSG( window.size() % 2 == 0, "Even window sizes expected." );

    using window_t = double;

    window_t const sizef   ( convert<window_t>( window.size() ) );
    window_t const halfSize( sizef / 2                          );
    window_t       i       ( 0                                  );
    window_t       w       ( 0                                  );
    window_t const dw      ( twoPi_d / sizef                    );

    switch ( windowType )
    {
        case Window::Rectangle: Math::fill( window, 1.0f );
                                break; // Rectangle

        case Window::Triangle : for ( ; i < halfSize; ++i ) { *pWindow++ = 2 *       i / sizef  ; }
                                BOOST_ASSERT( i == round( halfSize ) );
                                for ( ; i < sizef   ; ++i ) { *pWindow++ = 2 * ( 1 - i / sizef ); }
                                break; // Triangle

        case Window::Gaussian : while ( pWindow != pEnd )
                                {
                                    window_t const alpha( 3 );
                                    window_t const temp ( alpha * ( i++ - halfSize ) / halfSize );
                                    *pWindow++ = Math::exp( - 0.5 * temp * temp );
                                }
                                break; // Gaussian

        // Implementation note:
        // Hamming has COLA for 50% overlap and 75% overlap.
        //                                    (03.02.2010.) (Danijel Domazet)
        case Window::Hamming  : while ( pWindow != pEnd )
                                {
                                    *pWindow++ = 0.54 - 0.46 * std::cos( w );
                                    w += dw;
                                }
                                break; // Hamming

        case Window::Hann     : while ( pWindow != pEnd )
                                {
                                    *pWindow++ = 0.5 * ( 1 - std::cos( w ) );
                                    w += dw;
                                }
                                break; // Hann (COLA for (M+1)/2 overlap)

        case Window::FlatTop  : while ( pWindow != pEnd )
                                {
                                #if 1
                                    window_t const a0( 0.215578948 );
                                    window_t const a1( 0.416631580 );
                                    window_t const a2( 0.277263158 );
                                    window_t const a3( 0.083578947 );
                                    window_t const a4( 0.006947368 );
                                #else
                                    window_t const a0( 0.1881  );
                                    window_t const a1( 0.36923 );
                                    window_t const a2( 0.28702 );
                                    window_t const a3( 0.13077 );
                                    window_t const a4( 0.02488 );
                                #endif
                                    *pWindow++ = a0                     -
                                                 a1 * std::cos( 1 * w ) +
                                                 a2 * std::cos( 2 * w ) -
                                                 a3 * std::cos( 3 * w ) +
                                                 a4 * std::cos( 4 * w ) ;

                                    w += dw;
                                }
                                break; //Flat top

        case Window::Welch    : while ( pWindow != pEnd )
                                {
                                    window_t const temp( ( i++ - halfSize ) / halfSize );
                                    *pWindow++ = 1 - temp * temp;
                                }
                                break; // Welch

        case Window::Blackman : while ( pWindow != pEnd )
                                {
                                    window_t const a ( 0.16 );
                                    window_t const a0( ( 1 - a ) / 2 );
                                    window_t const a1( 1.0       / 2 );
                                    window_t const a2( a         / 2 );

                                    *pWindow++ = a0                     -
                                                 a1 * std::cos( 1 * w ) +
                                                 a2 * std::cos( 2 * w );

                                    w += dw;
                                }
                                /// \note Even the double precision floating
                                /// point cannot store the value of a0 (0.48)
                                /// with full precision which causes a tiny
                                /// negative value to be produced for the first
                                /// sample instead of an exact zero.
                                ///           (25.04.2012.) (Domagoj Saric)
                                window.front() = 0;
                                break; // Blackman

        case Window::BlackmanHarris:
                                while ( pWindow != pEnd )
                                {
                                    // Minimum 4-term Blackman-Harris window:

                                    window_t const a0( 0.35875 );
                                    window_t const a1( 0.48829 );
                                    window_t const a2( 0.14128 );
                                    window_t const a3( 0.01168 );

                                    *pWindow++ = a0                     -
                                                 a1 * std::cos( 1 * w ) +
                                                 a2 * std::cos( 2 * w ) -
                                                 a3 * std::cos( 3 * w );

                                    w += dw;
                                }
                                break; //Blackman Harris

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    /// \todo Research for windows that are COLA for some high overlap factors,
    ///       like 75% and more. Implementing windows that are not COLA makes
    ///       no sense.
    ///                                       (04.02.2010.) (Danijel Domazet)

    // Implementation note:
    //   Hann and Hamming windows satisfy the above criteria, but the
    // problem is that they satisfy it for integer overlap factors.
    // Unfortunately 2^n / {3, 5, 6, 7} is never an integer so the hop
    // size is truncated to the nearest int. One possibility is to have
    // only 2^n overlap factors, and the other one is to have overlap
    // factor - framesize pairs, eg hopsize 7 requires framesize 2058
    // (294*7 = 2058, deliberately took the nearest EVEN hop size that
    // satisfies the integer criteria). Of course, having non-power-of-two
    // frame sizes has its own drawbacks.
    //                                        (19.02.2010.) (Ivan Dokmanic)

    /// \todo Research the Vorbis window:
    ///     y = sin( 0.5 * Pi * sin^2((x+0.5)/n * Pi) )
    /// mentioned here
    /// http://www.hydrogenaudio.org/forums//lofiversion/index.php/t67700.html.
    ///                                       (16.04.2012.) (Domagoj Saric)

    // Verify the DFT-even requirement.
    BOOST_ASSERT( window[ 0 ] != window.back() || windowType == Window::Rectangle );
    BOOST_ASSERT( window[ 1 ] == window.back()                                    );

    BOOST_ASSERT_MSG( max( window ) > 0, "Non strictly positive window." );
}

#pragma warning( pop )

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
