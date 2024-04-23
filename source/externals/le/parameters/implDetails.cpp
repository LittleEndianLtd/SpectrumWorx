////////////////////////////////////////////////////////////////////////////////
///
/// \file implDetails.cpp
/// ---------------------
///
/// \internal
///
/// \brief Various implementation details and notes that we do not want exposed
/// in the SW SDK.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "le/math/conversion.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

/// \todo Add more extensive documentation for the Parameter class template and
/// the related architecture.
///                                           (11.03.2011.) (Domagoj Saric)

// Implementation note:
//   A Boost.Parameter based implementation (for the 'named' parameter feature)
// was investigated but it turned out it incurred a too large compile-time hit
// (it did however provide additional features, such as checking the correctness
// of passed parameters).
//                                            (01.04.2011.) (Domagoj Saric)

// Implementation note:
//   Up to revision 3753 an additional ExtendedParameters class template was
// provided that enabled inheriting from/expanding existing Parameters with the
// ability to filter which of the inherited parameters to forward as 'public'/
// UI parameters but it was since removed as parts of its functionality were no
// longer used while others were replaced with a different approach that has
// less compile time overhead.
//                                            (17.03.2011.) (Domagoj Saric)

/// \internal
namespace Detail
{
    // Implementation note:
    //   In order to prevent the inclusion of Math headers in parameter related
    // headers that have to be publicly exposed in the SDK, we declare a
    // a forwarder function in the Detail namespace which is then implemented
    // in a hidden .cpp and exported as necessary.
    //                                        (18.04.2011.) (Domagoj Saric)
    template <typename T>
    bool isValueInRange( T const value, T const rangeMinimum, T const rangeMaximum )
    {
        return Math::isValueInRange( value, rangeMinimum, rangeMaximum );
    }

    template bool isValueInRange<float        >( float         value, float         rangeMinimum, float         rangeMaximum );
    template bool isValueInRange<int          >( int           value, int           rangeMinimum, int           rangeMaximum );
    template bool isValueInRange<unsigned int >( unsigned int  value, unsigned int  rangeMinimum, unsigned int  rangeMaximum );
    template bool isValueInRange<std:: int16_t>( std:: int16_t value, std:: int16_t rangeMinimum, std:: int16_t rangeMaximum );
    template bool isValueInRange<std::uint16_t>( std::uint16_t value, std::uint16_t rangeMinimum, std::uint16_t rangeMaximum );
    template bool isValueInRange<std::uint8_t >( std::uint8_t  value, std::uint8_t  rangeMinimum, std::uint8_t  rangeMaximum );

    template <> bool isValueInRange<float const &>( float const & value, float const & rangeMinimum, float const & rangeMaximum )
    {
        return isValueInRange<float>( value, rangeMinimum, rangeMaximum );
    }
}  // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
