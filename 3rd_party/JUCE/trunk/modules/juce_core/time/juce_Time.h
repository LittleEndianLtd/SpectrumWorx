/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_TIME_H_INCLUDED
#define JUCE_TIME_H_INCLUDED


//==============================================================================
/**
    Holds an absolute date and time.

    Internally, the time is stored at millisecond precision.

    @see RelativeTime
*/
class JUCE_API  Time
{
public:
    //==============================================================================
    /** Creates a Time object.

        This default constructor creates a time of 1st January 1970, (which is
        represented internally as 0ms).

        To create a time object representing the current time, use getCurrentTime().

        @see getCurrentTime
    */
    Time() noexcept;

    /** Creates a time based on a number of milliseconds.

        The internal millisecond count is set to 0 (1st January 1970). To create a
        time object set to the current time, use getCurrentTime().

        @param millisecondsSinceEpoch   the number of milliseconds since the unix
                                        'epoch' (midnight Jan 1st 1970).
        @see getCurrentTime, currentTimeMillis
    */
    explicit Time (int64 millisecondsSinceEpoch) noexcept;

    /** Creates a time from a set of date components.

        The timezone is assumed to be whatever the system is using as its locale.

        @param year             the year, in 4-digit format, e.g. 2004
        @param month            the month, in the range 0 to 11
        @param day              the day of the month, in the range 1 to 31
        @param hours            hours in 24-hour clock format, 0 to 23
        @param minutes          minutes 0 to 59
        @param seconds          seconds 0 to 59
        @param milliseconds     milliseconds 0 to 999
        @param useLocalTime     if true, encode using the current machine's local time; if
                                false, it will always work in GMT.
    */
    Time (int year,
          int month,
          int day,
          int hours,
          int minutes,
          int seconds = 0,
          int milliseconds = 0,
          bool useLocalTime = true) noexcept;

    /** Creates a copy of another Time object. */
    Time (const Time& other) noexcept;

    /** Destructor. */
    JUCE_ORIGINAL( ~Time() noexcept; )

    /** Copies this time from another one. */
    Time& operator= (const Time& other) noexcept;

    //==============================================================================
    /** Returns a Time object that is set to the current system time.

        @see currentTimeMillis
    */
    static Time JUCE_CALLTYPE getCurrentTime() noexcept;

    /** Returns the time as a number of milliseconds.

        @returns    the number of milliseconds this Time object represents, since
                    midnight jan 1st 1970.
        @see getMilliseconds
    */
    int64 toMilliseconds() const noexcept                           { return millisSinceEpoch; }

    /** Returns the year.

        A 4-digit format is used, e.g. 2004.
    */
    int getYear() const noexcept;

    /** Returns the number of the month.

        The value returned is in the range 0 to 11.
        @see getMonthName
    */
    int getMonth() const noexcept;

    /** Returns the name of the month.

        @param threeLetterVersion   if true, it'll be a 3-letter abbreviation, e.g. "Jan"; if false
                                    it'll return the long form, e.g. "January"
        @see getMonth
    */
    String getMonthName (bool threeLetterVersion) const;

    /** Returns the day of the month.
        The value returned is in the range 1 to 31.
    */
    int getDayOfMonth() const noexcept;

    /** Returns the number of the day of the week.
        The value returned is in the range 0 to 6 (0 = sunday, 1 = monday, etc).
    */
    int getDayOfWeek() const noexcept;

    /** Returns the number of the day of the year.
        The value returned is in the range 0 to 365.
    */
    int getDayOfYear() const noexcept;

    /** Returns the name of the weekday.

        @param threeLetterVersion   if true, it'll return a 3-letter abbreviation, e.g. "Tue"; if
                                    false, it'll return the full version, e.g. "Tuesday".
    */
    String getWeekdayName (bool threeLetterVersion) const;

    /** Returns the number of hours since midnight.

        This is in 24-hour clock format, in the range 0 to 23.

        @see getHoursInAmPmFormat, isAfternoon
    */
    int getHours() const noexcept;

    /** Returns true if the time is in the afternoon.

        So it returns true for "PM", false for "AM".

        @see getHoursInAmPmFormat, getHours
    */
    bool isAfternoon() const noexcept;

    /** Returns the hours in 12-hour clock format.

        This will return a value 1 to 12 - use isAfternoon() to find out
        whether this is in the afternoon or morning.

        @see getHours, isAfternoon
    */
    int getHoursInAmPmFormat() const noexcept;

    /** Returns the number of minutes, 0 to 59. */
    int getMinutes() const noexcept;

    /** Returns the number of seconds, 0 to 59. */
    int getSeconds() const noexcept;

    /** Returns the number of milliseconds, 0 to 999.

        Unlike toMilliseconds(), this just returns the position within the
        current second rather than the total number since the epoch.

        @see toMilliseconds
    */
    int getMilliseconds() const noexcept;

    /** Returns true if the local timezone uses a daylight saving correction. */
    bool isDaylightSavingTime() const noexcept;

    /** Returns a 3-character string to indicate the local timezone. */
    String getTimeZone() const noexcept;

    //==============================================================================
    /** Quick way of getting a string version of a date and time.

        For a more powerful way of formatting the date and time, see the formatted() method.

        @param includeDate      whether to include the date in the string
        @param includeTime      whether to include the time in the string
        @param includeSeconds   if the time is being included, this provides an option not to include
                                the seconds in it
        @param use24HourClock   if the time is being included, sets whether to use am/pm or 24
                                hour notation.
        @see formatted
    */
    String toString (bool includeDate,
                     bool includeTime,
                     bool includeSeconds = true,
                     bool use24HourClock = false) const noexcept;

    /** Converts this date/time to a string with a user-defined format.

        This uses the C strftime() function to format this time as a string. To save you
        looking it up, these are the escape codes that strftime uses (other codes might
        work on some platforms and not others, but these are the common ones):

        %a  is replaced by the locale's abbreviated weekday name.
        %A  is replaced by the locale's full weekday name.
        %b  is replaced by the locale's abbreviated month name.
        %B  is replaced by the locale's full month name.
        %c  is replaced by the locale's appropriate date and time representation.
        %d  is replaced by the day of the month as a decimal number [01,31].
        %H  is replaced by the hour (24-hour clock) as a decimal number [00,23].
        %I  is replaced by the hour (12-hour clock) as a decimal number [01,12].
        %j  is replaced by the day of the year as a decimal number [001,366].
        %m  is replaced by the month as a decimal number [01,12].
        %M  is replaced by the minute as a decimal number [00,59].
        %p  is replaced by the locale's equivalent of either a.m. or p.m.
        %S  is replaced by the second as a decimal number [00,61].
        %U  is replaced by the week number of the year (Sunday as the first day of the week) as a decimal number [00,53].
        %w  is replaced by the weekday as a decimal number [0,6], with 0 representing Sunday.
        %W  is replaced by the week number of the year (Monday as the first day of the week) as a decimal number [00,53]. All days in a new year preceding the first Monday are considered to be in week 0.
        %x  is replaced by the locale's appropriate date representation.
        %X  is replaced by the locale's appropriate time representation.
        %y  is replaced by the year without century as a decimal number [00,99].
        %Y  is replaced by the year with century as a decimal number.
        %Z  is replaced by the timezone name or abbreviation, or by no bytes if no timezone information exists.
        %%  is replaced by %.

        @see toString
    */
    String formatted (const String& format) const;

    //==============================================================================
    /** Adds a RelativeTime to this time. */
    Time& operator+= (RelativeTime delta);
    /** Subtracts a RelativeTime from this time. */
    Time& operator-= (RelativeTime delta);

    //==============================================================================
    /** Tries to set the computer's clock.

        @returns    true if this succeeds, although depending on the system, the
                    application might not have sufficient privileges to do this.
    */
    bool setSystemTimeToThisTime() const;

    //==============================================================================
    /** Returns the name of a day of the week.

        @param dayNumber            the day, 0 to 6 (0 = sunday, 1 = monday, etc)
        @param threeLetterVersion   if true, it'll return a 3-letter abbreviation, e.g. "Tue"; if
                                    false, it'll return the full version, e.g. "Tuesday".
    */
    static String getWeekdayName (int dayNumber,
                                  bool threeLetterVersion);

    /** Returns the name of one of the months.

        @param monthNumber  the month, 0 to 11
        @param threeLetterVersion   if true, it'll be a 3-letter abbreviation, e.g. "Jan"; if false
                                    it'll return the long form, e.g. "January"
    */
    static String getMonthName (int monthNumber,
                                bool threeLetterVersion);

    //==============================================================================
    // Static methods for getting system timers directly..

    /** Returns the current system time.

        Returns the number of milliseconds since midnight jan 1st 1970.

        Should be accurate to within a few millisecs, depending on platform,
        hardware, etc.
    */
    static int64 currentTimeMillis() noexcept;

    /** Returns the number of millisecs since a fixed event (usually system startup).

        This returns a monotonically increasing value which it unaffected by changes to the
        system clock. It should be accurate to within a few millisecs, depending on platform,
        hardware, etc.

        Being a 32-bit return value, it will of course wrap back to 0 after 2^32 seconds of
        uptime, so be careful to take that into account. If you need a 64-bit time, you can
        use currentTimeMillis() instead.

        @see getApproximateMillisecondCounter
    */
    static uint32 getMillisecondCounter() noexcept;

    /** Returns the number of millisecs since a fixed event (usually system startup).

        This has the same function as getMillisecondCounter(), but returns a more accurate
        value, using a higher-resolution timer if one is available.

        @see getMillisecondCounter
    */
    static double getMillisecondCounterHiRes() noexcept;

    /** Waits until the getMillisecondCounter() reaches a given value.

        This will make the thread sleep as efficiently as it can while it's waiting.
    */
    static void waitForMillisecondCounter (uint32 targetTime) noexcept;

    /** Less-accurate but faster version of getMillisecondCounter().

        This will return the last value that getMillisecondCounter() returned, so doesn't
        need to make a system call, but is less accurate - it shouldn't be more than
        100ms away from the correct time, though, so is still accurate enough for a
        lot of purposes.

        @see getMillisecondCounter
    */
    static uint32 getApproximateMillisecondCounter() noexcept;

    //==============================================================================
    // High-resolution timers..

    /** Returns the current high-resolution counter's tick-count.

        This is a similar idea to getMillisecondCounter(), but with a higher
        resolution.

        @see getHighResolutionTicksPerSecond, highResolutionTicksToSeconds,
             secondsToHighResolutionTicks
    */
    static int64 getHighResolutionTicks() noexcept;

    /** Returns the resolution of the high-resolution counter in ticks per second.

        @see getHighResolutionTicks, highResolutionTicksToSeconds,
             secondsToHighResolutionTicks
    */
    static int64 getHighResolutionTicksPerSecond() noexcept;

    /** Converts a number of high-resolution ticks into seconds.

        @see getHighResolutionTicks, getHighResolutionTicksPerSecond,
             secondsToHighResolutionTicks
    */
    static double highResolutionTicksToSeconds (int64 ticks) noexcept;

    /** Converts a number seconds into high-resolution ticks.

        @see getHighResolutionTicks, getHighResolutionTicksPerSecond,
             highResolutionTicksToSeconds
    */
    static int64 secondsToHighResolutionTicks (double seconds) noexcept;


private:
    //==============================================================================
    int64 millisSinceEpoch;
};

//==============================================================================
/** Adds a RelativeTime to a Time. */
JUCE_API Time operator+ (Time time, RelativeTime delta);
/** Adds a RelativeTime to a Time. */
JUCE_API Time operator+ (RelativeTime delta, Time time);

/** Subtracts a RelativeTime from a Time. */
JUCE_API Time operator- (Time time, RelativeTime delta);
/** Returns the relative time difference between two times. */
JUCE_API const RelativeTime operator- (Time time1, Time time2);

/** Compares two Time objects. */
JUCE_API bool operator== (Time time1, Time time2);
/** Compares two Time objects. */
JUCE_API bool operator!= (Time time1, Time time2);
/** Compares two Time objects. */
JUCE_API bool operator<  (Time time1, Time time2);
/** Compares two Time objects. */
JUCE_API bool operator<= (Time time1, Time time2);
/** Compares two Time objects. */
JUCE_API bool operator>  (Time time1, Time time2);
/** Compares two Time objects. */
JUCE_API bool operator>= (Time time1, Time time2);


#endif   // JUCE_TIME_H_INCLUDED
