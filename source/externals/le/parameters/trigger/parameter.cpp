////////////////////////////////////////////////////////////////////////////////
///
/// \file trigger/parameter.cpp
/// ---------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "parameter.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class TriggerParameter
///
/// \brief An auto-reset boolean parameter that models a push-button-like
/// control (a "biased switch"
/// http://en.wikipedia.org/wiki/Switch#Biased_switches).
///
/// In order to ensure that each 'button push' (an external setValue( true )
/// call) gets detected/'consumed' and because there is an arbitrary delay
/// between the moment the control and the TriggerParameter get activated and
/// the moment the value of the parameter's value/state gets examined, the
/// TriggerParameter class will delay reseting itself (after an external
/// setValue( false ) call) until the previously set value is consumed.
///
/// Because certain functionality (such as preset saving and host notifications)
/// only need to inspect the current value of a parameter (and expect this to be
/// a truly immutable operation) the TriggerParameter class introduces the
/// consumeValue() member function that performs the previously described
/// auto-reset behaviour (of returning the current value and then resetting the
/// parameter if setValue( false ) was called) while the getValue() member
/// function still has the same semantics inherited from the Boolean parameter
/// class.
///
/// As a consequence of the above described semantics it is no longer clear how
/// should the automatic value_type conversion operator and the increment and
/// decrement operators behave so they are disabled, forcing the user to
/// explicitly state what he or she wants to do with the parameter (by calling
/// either getValue() or consumeValue()).
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// TriggerParameter::consumeValue()
// --------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// Returns the current value of the parameter and resets it.
///
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS
TriggerParameter::value_type TriggerParameter::consumeValue() const
{
    value_type const currentValue( getValue() );
    const_cast<TriggerParameter &>( *this ).reset();
    return currentValue;
}


////////////////////////////////////////////////////////////////////////////////
//
// TriggerParameter::setValue()
// ----------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// In order to guarantee that each 'trigger' (setting the parameter to true)
/// gets detected/read/used setValue() is only allowed to set the value to true.
/// The parameter is then reset (set to false) after the call to consumeValue().
///
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS
void TriggerParameter::setValue( param_type const value )
{
    value_ |= value;
}

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
