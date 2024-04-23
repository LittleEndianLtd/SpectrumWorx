////////////////////////////////////////////////////////////////////////////////
///
/// historyBuffer.cpp
/// -----------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "historyBuffer.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

ReversedHistoryBufferState::HistoryData ReversedHistoryBufferState::getCurrentStepData
(
    std::uint16_t const historyLengthInSteps,
    std::uint16_t const numberOfBins,
    DataRange     const historyData
)
{
    // Implementation note:
    //   The step counter has to be incremented at the beginning/before actual
    // processing because the step counter incrementing functionality (below)
    // also automatically adjusts it for any changes the user might have made to
    // historyLengthInSteps and/or the engine setup. If this was not done so an
    // access-out-of-bounds could happen when the user switches to a larger
    // FFT size (so the value of numberOfBins becomes large) and the value of
    // step_ remains unadjusted/high from the previous setup (when lots of small
    // frames were used), then the value of 'step_ * numberOfBins' would 'blow
    // up'.
    //                                        (21.05.2010.) (Domagoj Saric)
    step_ += increment_;

    // Implementation note:
    //   After the above increment the counter might have gone outside the
    // [0, historyLengthInSteps) range but because it is of unsigned type an
    // overflown result will thus always be >= historyLengthInSteps.
    //                                        (21.05.2010.) (Domagoj Saric)
    if ( step_ >= historyLengthInSteps )
    {
        // Implementation note:
        //   Ideally we could now just do:
        //  increment_  = - increment_;
        //  step_      +=   increment_;
        // and the counter would be reset to either zero or
        // historyLengthInSteps - 1 (because step_ would either be -1 or
        // historyLengthInSteps) but because the user might have changed
        // historyLengthInSteps during processing (and thus reset(), usually in
        // a ChannelState::reset() function, would not have been called) step_
        // might have some indeterminate value (e.g. much larger than
        // historyLengthInSteps if the user just decreased
        // historyLengthInSteps). For this reason we need to perform more
        // complex logic and properly (re)set step_.
        //                                    (21.05.2010.) (Domagoj Saric)
        if ( increment_ > 0 )
        {
            LE_ASSUME( increment_ == 1 );
            increment_ = - 1;
            step_      = historyLengthInSteps - 1;
        }
        else
        {
            LE_ASSUME( increment_ == -1 );
            // Implementation note:
            //   Here we check for another special case: the fact that our
            // current position (step_) went outside the 'recorded' history
            // range while moving downwards/to the left does not necessarily
            // mean that we have reached and passed the lower bound of history
            // buffer (step 0) and that we must now change the traversal
            // direction and go upwards, it can also be that the user/LFO just
            // decreased lengthInSteps below our current position. In the latter
            // case we must not change the traversal direction because that
            // would result in non-reversed output (as the current traversal
            // direction is the correct one, i.e. reversed), we must only skip
            // over to the closest (the rightmost/uppermost) valid step.
            //                                (08.06.2010.) (Domagoj Saric)
            if ( actualHistoryLengthInSteps_ > historyLengthInSteps )
            {
                step_ = historyLengthInSteps - 1;
            }
            else
            {
                increment_ = 1;
                step_      = 0;
            }
        }
    }

    BOOST_ASSERT_MSG( step_ < historyLengthInSteps, "Step overflow." );

    unsigned int const numberOfBinsAligned( Math::alignIndex( numberOfBins ) );
    unsigned int const fullFrameSize      ( numberOfBinsAligned * 2     );

    float * const pTargetAmplitudesOrReals( &historyData[ step_ * fullFrameSize ]          );
    float * const pTargetPhasesOrImags    ( pTargetAmplitudesOrReals + numberOfBinsAligned );

    float *       pSourceAmplitudesOrReals( pTargetAmplitudesOrReals                       );
    float *       pSourcePhasesOrImags    ( pTargetPhasesOrImags                           );

    // Implementation note:
    //   If the current (desired) reversing length is longer than the history we
    // have available and we have 'stepped-out' of available history we do not
    // want silence or garbage/ancient leftover history to be played out.
    // Instead we "emulate history" by looping along the available history,
    // playing it out until enough history has been gathered to continue normal
    // operation.
    //                                        (08.06.2010.) (Domagoj Saric)
    if ( actualHistoryLengthInSteps_ < historyLengthInSteps )
    {
        if ( step_ >= actualHistoryLengthInSteps_ )
        {
            pSourceAmplitudesOrReals = &historyData[ ( step_ - emulatedHistoryStepOffset_ ) * fullFrameSize ];
            pSourcePhasesOrImags     = pSourceAmplitudesOrReals + numberOfBinsAligned;

            // Implementation note:
            //   We have to move the emulated history 'pointer' one point back
            // in time but since it is actually a relative offset we have to
            // increase it by two (instead of one) because the value of step_
            // from which it is subtracted will be incremented in the meantime
            // thus the value of historyStep will be less exactly by one in the
            // next iteration.
            //                                (08.06.2010.) (Domagoj Saric)
            ++emulatedHistoryStepOffset_;
            ++emulatedHistoryStepOffset_;

            ++actualHistoryLengthInSteps_;

            if ( emulatedHistoryStepOffset_ > actualHistoryLengthInSteps_ )
                emulatedHistoryStepOffset_ = 0;
        }
    }
    else
    {
        // Reset the 'history emulation state' (should actually only be done
        // once after a 'history underrun' situation has passed).
        actualHistoryLengthInSteps_ = historyLengthInSteps;
        emulatedHistoryStepOffset_  = 0                   ;
    }

    HistoryData const result =
    {
        { pTargetAmplitudesOrReals, pTargetPhasesOrImags },
        { pSourceAmplitudesOrReals, pSourcePhasesOrImags }        
    };

    return result;
}


void ReversedHistoryBufferState::reset()
{
    // Implementation note:
    //   Because the step_ is incremented at the beginning of the process()
    // function we have to set it to -1 here so that it would become 0 when the
    // actual processing starts.
    //                                        (21.05.2010.) (Domagoj Saric)
    step_      = static_cast<decltype( step_ )>( -1 );
    increment_ = 1;

    actualHistoryLengthInSteps_ = 0;
    emulatedHistoryStepOffset_  = 0;
}


bool ReversedHistoryBufferState::HistoryData::isEmulated() const
{
    return targetHistory.pAmplitudesOrReals != sourceHistory.pAmplitudesOrReals;
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
