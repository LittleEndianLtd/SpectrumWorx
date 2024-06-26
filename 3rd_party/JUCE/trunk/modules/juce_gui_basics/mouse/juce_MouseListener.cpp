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

void MouseListener::mouseEnter (const MouseEvent&) {}
void MouseListener::mouseExit (const MouseEvent&) LE_PATCH( noexcept )  {}
void MouseListener::mouseDown (const MouseEvent&)  {}
void MouseListener::mouseUp (const MouseEvent&)    {}
void MouseListener::mouseDrag (const MouseEvent&)  {}
void MouseListener::mouseMove (const MouseEvent&) LE_PATCH( noexcept )  {}
void MouseListener::mouseDoubleClick (const MouseEvent&) {}
void MouseListener::mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) LE_PATCH( noexcept ) {}
