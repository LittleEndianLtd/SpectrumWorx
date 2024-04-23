////////////////////////////////////////////////////////////////////////////////
///
/// \file spectrumWorxSharedImpl.hpp
/// --------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef spectrumWorxSharedImpl_hpp__41090002_3305_4404_9DB4_FCAEFB31B564
#define spectrumWorxSharedImpl_hpp__41090002_3305_4404_9DB4_FCAEFB31B564
#pragma once
//------------------------------------------------------------------------------
#if LE_SW_SEPARATED_DSP_GUI || !LE_SW_GUI
    #include "core/spectrumWorxCore.hpp"
#else
    #include "spectrumWorx.hpp"
#endif

#include "core/host_interop/host2PluginImpl.hpp"
#include "core/host_interop/plugin2HostImpl.hpp"
#include "le/plugins/plugin.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class SpectrumWorxSharedImpl
///
/// \brief Main SpectrumWorx plugin implementation class. Functionality shared
/// for all protocols.
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

template <class Impl, class Protocol>
class LE_NOVTABLE SpectrumWorxSharedImpl
    :
    public Plugins::Plugin                       <Impl, Protocol              >,
    public SW     ::Host2PluginInteropImpl       <Impl, Protocol              >,
    public SW     ::Plugin2HostPassiveInteropImpl<Impl, Protocol              >,
#if !LE_SW_GUI || LE_SW_SEPARATED_DSP_GUI
    public SpectrumWorxCore
#else
    public SW     ::Plugin2HostActiveInteropImpl <Impl, Protocol, SpectrumWorx>
#endif // LE_SW_SEPARATED_DSP_GUI
{
protected:
#if !LE_SW_GUI || LE_SW_SEPARATED_DSP_GUI
    typedef SpectrumWorxCore Base;
#else
    typedef Plugin2HostActiveInteropImpl<Impl, Protocol, SpectrumWorx> Base;
#endif // !LE_SW_GUI || LE_SW_SEPARATED_DSP_GUI

#if LE_SW_ENGINE_INPUT_MODE >= 1
    typedef SpectrumWorxCore::InputMode     InputMode    ;
#endif // LE_SW_ENGINE_INPUT_MODE >= 1
    typedef SpectrumWorxCore::MixPercentage MixPercentage;
    typedef SpectrumWorxCore::Parameters    Parameters   ;

    typedef typename Plugins::ErrorCode<Protocol>::value_type ErrorCode;

public:
    typedef Plugins::Plugin<Impl, Protocol>                         PluginPlatform    ;
    typedef typename Plugins::AutomatedParameterFor<Protocol>::type AutomatedParameter;

public: // Plugin framework interface
    explicit SpectrumWorxSharedImpl( typename PluginPlatform::ConstructionParameter );

    //...mrmlj...
    using Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameterDisplay;
    using Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameterLabel;
    using Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameterName;
    using Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameterProperties;
    using Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameter;
    using Host2PluginInteropImpl       <Impl, Protocol>::setParameter;

    using Host2PluginInteropImpl       <Impl, Protocol>::isHost; //...mrmlj...

    friend class Host2PluginInteropImpl<Impl, Protocol>;

    ErrorCode LE_NOTHROW initialise();

    void LE_NOTHROW process( float const * const * inputs, float * * outputs, std::uint32_t samples );

#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
    typedef SpectrumWorx::Editor Editor;

    bool createGUI( typename PluginPlatform::Editor::WindowHandle const parentWindow )
    {
        // Implementation note:
        //   Some hosts (e.g. Sonar 8) are unable to give the timing information
        // while inside a plugin's constructor so we make the call here, on GUI
        // creation, as it is only needed for correct LFODisplay appearance if
        // the timing information has not already been gathered (the plugin's
        // process() function has not yet been called).
        //                                    (28.01.2011.) (Domagoj Saric)
        updateTimingInformation();
        if ( SpectrumWorx::createGUI() )
        {
            this->gui()->attachToHostWindow( parentWindow );
            return true;
        }
        return false;
    }
#endif // LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI

public:
    using Base::setGlobalParameter;

protected:
    using PluginPlatform::impl;

#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
    ////////////////////////////////////////////////////////////////////////////
    // Programs and presets
    ////////////////////////////////////////////////////////////////////////////

    public:
        using Base::setProgramName;
        using Base::getProgramName;
        using Base::saveProgramState;
        using Base::loadProgramState;

    //private:...mrmlj...
        void markCurrentProgramAsModified() const
        {
            typename PluginPlatform::ProgramNameBuf & programName( const_cast<typename PluginPlatform::ProgramNameBuf &>( reinterpret_cast<typename PluginPlatform::ProgramNameBuf const &>( this->program().name() ) ) );
            if ( programName[ 0 ] != '*' )
            {
                std::memmove( &programName[ 1 ], &programName[ 0 ], sizeof( programName ) - 1 );
                programName[                         0 ] = '*';
                programName[ sizeof( programName ) - 1 ] = 0  ;

                this->host().currentPresetNameChanged();
            }
        }


    ////////////////////////////////////////////////////////////////////////////
    // Timing information
    ////////////////////////////////////////////////////////////////////////////

    private:
        LE_NOTHROW bool updateTimingInformation();
#endif // LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
}; // class SpectrumWorxSharedImpl

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // spectrumWorxSharedImpl_hpp
