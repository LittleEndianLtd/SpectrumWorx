/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::parent ("parent");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::left ("left");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::right ("right");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::top ("top");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::bottom ("bottom");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::x ("x");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::y ("y");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::width ("width");
LE_PATCH( LE_WEAK_SYMBOL ) const String RelativeCoordinate::Strings::height ("height");

RelativeCoordinate::StandardStrings::Type RelativeCoordinate::StandardStrings::getTypeOf (const String& s) noexcept
{
    if (s == Strings::left)    return left;
    if (s == Strings::right)   return right;
    if (s == Strings::top)     return top;
    if (s == Strings::bottom)  return bottom;
    if (s == Strings::x)       return x;
    if (s == Strings::y)       return y;
    if (s == Strings::width)   return width;
    if (s == Strings::height)  return height;
    if (s == Strings::parent)  return parent;
    return unknown;
}

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
{
}

#ifndef LE_PATCHED_JUCE
RelativeCoordinate::RelativeCoordinate (const Expression& term_)
    : term (term_)
{
}
#endif // LE_PATCHED_JUCE

RelativeCoordinate::RelativeCoordinate (const RelativeCoordinate& other)
#ifndef LE_PATCHED_JUCE
    : term (other.term)
#endif // LE_PATCHED_JUCE
{
}

RelativeCoordinate& RelativeCoordinate::operator= (const RelativeCoordinate& other)
{
#ifndef LE_PATCHED_JUCE
    term = other.term;
#endif // LE_PATCHED_JUCE
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
RelativeCoordinate::RelativeCoordinate (RelativeCoordinate&& other) noexcept
#ifndef LE_PATCHED_JUCE
    : term (static_cast <Expression&&> (other.term))
#endif // LE_PATCHED_JUCE
{
}

RelativeCoordinate& RelativeCoordinate::operator= (RelativeCoordinate&& other) noexcept
{
#ifndef LE_PATCHED_JUCE
    term = static_cast <Expression&&> (other.term);
#endif // LE_PATCHED_JUCE
    return *this;
}
#endif

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin)
#ifndef LE_PATCHED_JUCE
    : term (absoluteDistanceFromOrigin)
#endif // LE_PATCHED_JUCE
{
}

#ifndef LE_PATCHED_JUCE
RelativeCoordinate::RelativeCoordinate (const String& s)
{
    try
    {
        term = Expression (s);
    }
    catch (Expression::ParseError&)
    {}
}
#endif // LE_PATCHED_JUCE

RelativeCoordinate::~RelativeCoordinate() LE_PATCH( noexcept )
{
}

#ifndef LE_PATCHED_JUCE
bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const noexcept
{
    return term.toString() == other.term.toString();
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const noexcept
{
    return ! operator== (other);
}

double RelativeCoordinate::resolve (const Expression::Scope* scope) const
{
    try
    {
        if (scope != nullptr)
            return term.evaluate (*scope);

        return term.evaluate();
    }
    catch (Expression::ParseError&)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const Expression::Scope* scope) const
{
    try
    {
        if (scope != nullptr)
            term.evaluate (*scope);
        else
            term.evaluate();
    }
    catch (Expression::ParseError&)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const Expression::Scope* scope)
{
    try
    {
        if (scope != nullptr)
        {
            term = term.adjustedToGiveNewResult (newPos, *scope);
        }
        else
        {
            Expression::Scope defaultScope;
            term = term.adjustedToGiveNewResult (newPos, defaultScope);
        }
    }
    catch (Expression::ParseError&)
    {}
}
#endif // LE_PATCHED_JUCE

bool RelativeCoordinate::isDynamic() const
{
#ifdef LE_PATCHED_JUCE
    return false;
#else
    return term.usesAnySymbols();
#endif // LE_PATCHED_JUCE
}

#ifndef LE_PATCHED_JUCE
String RelativeCoordinate::toString() const
{
    return term.toString();
}
#endif // LE_PATCHED_JUCE
