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

#ifndef JUCE_EDGETABLE_H_INCLUDED
#define JUCE_EDGETABLE_H_INCLUDED


//==============================================================================
/**
    A table of horizontal scan-line segments - used for rasterising Paths.

    @see Path, Graphics
*/
class JUCE_API  EdgeTable
{
public:
    //==============================================================================
    /** Creates an edge table containing a path.

        A table is created with a fixed vertical range, and only sections of the path
        which lie within this range will be added to the table.

        @param clipLimits               only the region of the path that lies within this area will be added
        @param pathToAdd                the path to add to the table
        @param transform                a transform to apply to the path being added
    */
    EdgeTable (const Rectangle<int>& clipLimits,
               const Path& pathToAdd,
               const AffineTransform& transform);

    /** Creates an edge table containing a rectangle. */
    explicit EdgeTable (const Rectangle<int>& rectangleToAdd);

    /** Creates an edge table containing a rectangle list. */
    explicit EdgeTable (const RectangleList<int>& rectanglesToAdd);

    /** Creates an edge table containing a rectangle list. */
    explicit EdgeTable (const RectangleList<float>& rectanglesToAdd);

    /** Creates an edge table containing a rectangle. */
    explicit EdgeTable (const Rectangle<float>& rectangleToAdd);

    /** Creates a copy of another edge table. */
    EdgeTable (const EdgeTable& other);

    /** Copies from another edge table. */
    EdgeTable& operator= (const EdgeTable& other);

    /** Destructor. */
    ~EdgeTable();

    //==============================================================================
    void clipToRectangle (const Rectangle<int>& r);
    void excludeRectangle (const Rectangle<int>& r);
    void clipToEdgeTable (const EdgeTable& other);
    void clipLineToMask (int x, int y, const uint8* mask, int maskStride, int numPixels);
    bool isEmpty() noexcept;
    const Rectangle<int>& getMaximumBounds() const noexcept      { return bounds; }
    void translate (float dx, int dy) noexcept;

    /** Reduces the amount of space the table has allocated.

        This will shrink the table down to use as little memory as possible - useful for
        read-only tables that get stored and re-used for rendering.
    */
    void optimiseTable();


    //==============================================================================
    /** Iterates the lines in the table, for rendering.

        This function will iterate each line in the table, and call a user-defined class
        to render each pixel or continuous line of pixels that the table contains.

        @param iterationCallback    this templated class must contain the following methods:
                                        @code
                                        inline void setEdgeTableYPos (int y);
                                        inline void handleEdgeTablePixel (int x, int alphaLevel) const;
                                        inline void handleEdgeTablePixelFull (int x) const;
                                        inline void handleEdgeTableLine (int x, int width, int alphaLevel) const;
                                        inline void handleEdgeTableLineFull (int x, int width) const;
                                        @endcode
                                        (these don't necessarily have to be 'const', but it might help it go faster)
    */
    template <class EdgeTableIterationCallback>
    void iterate (EdgeTableIterationCallback& iterationCallback) const noexcept
    {
        const int* lineStart = table;

        for (int y = 0; y < bounds.getHeight(); ++y)
        {
            const int* line = lineStart;
            lineStart += lineStrideElements;
            int numPoints = line[0];

            if (--numPoints > 0)
            {
                int x = *++line;
                jassert ((x >> 8) >= bounds.getX() && (x >> 8) < bounds.getRight());
                int levelAccumulator = 0;

                iterationCallback.setEdgeTableYPos (bounds.getY() + y);

                while (--numPoints >= 0)
                {
                    const int level = *++line;
                    jassert (isPositiveAndBelow (level, (int) 256));
                    const int endX = *++line;
                    jassert (endX >= x);
                    const int endOfRun = (endX >> 8);

                    if (endOfRun == (x >> 8))
                    {
                        // small segment within the same pixel, so just save it for the next
                        // time round..
                        levelAccumulator += (endX - x) * level;
                    }
                    else
                    {
                        // plot the fist pixel of this segment, including any accumulated
                        // levels from smaller segments that haven't been drawn yet
                        levelAccumulator += (0x100 - (x & 0xff)) * level;
                        levelAccumulator >>= 8;
                        x >>= 8;

                        if (levelAccumulator > 0)
                        {
                            if (levelAccumulator >= 255)
                                iterationCallback.handleEdgeTablePixelFull (x);
                            else
                                iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                        }

                        // if there's a run of similar pixels, do it all in one go..
                        if (level > 0)
                        {
                            jassert (endOfRun <= bounds.getRight());
                            const int numPix = endOfRun - ++x;

                            if (numPix > 0)
                                iterationCallback.handleEdgeTableLine (x, numPix, level);
                        }

                        // save the bit at the end to be drawn next time round the loop.
                        levelAccumulator = (endX & 0xff) * level;
                    }

                    x = endX;
                }

                levelAccumulator >>= 8;

                if (levelAccumulator > 0)
                {
                    x >>= 8;
                    jassert (x >= bounds.getX() && x < bounds.getRight());

                    if (levelAccumulator >= 255)
                        iterationCallback.handleEdgeTablePixelFull (x);
                    else
                        iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                }
            }
        }
    }

private:
    //==============================================================================
    // table line format: number of points; point0 x, point0 levelDelta, point1 x, point1 levelDelta, etc
    struct LineItem   { int x, level; };
    struct LineSorter;

    HeapBlock<int> table;
    Rectangle<int> bounds;
    int maxEdgesPerLine, lineStrideElements;
    bool needToCheckEmptinesss;

    void allocate();
    void clearLineSizes() noexcept;
    void addEdgePoint (int x, int y, int winding);
    void addEdgePointPair (int x1, int x2, int y, int winding);
    void remapTableForNumEdges (int newNumEdgesPerLine);
    void intersectWithEdgeTableLine (int y, const int* otherLine);
    void clipEdgeTableLineToRange (int* line, int x1, int x2) noexcept;
    void sanitiseLevels (bool useNonZeroWinding) noexcept;
    static void copyEdgeTableData (int* dest, int destLineStride, const int* src, int srcLineStride, int numLines) noexcept;

    JUCE_LEAK_DETECTOR (EdgeTable)
};


#endif   // JUCE_EDGETABLE_H_INCLUDED
