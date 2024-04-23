////////////////////////////////////////////////////////////////////////////////
///
/// \file pimpl.hpp
/// ---------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pimpl_hpp__9B6D22C9_F9CA_45F8_BEFC_BD3D6199A2EC
#define pimpl_hpp__9B6D22C9_F9CA_45F8_BEFC_BD3D6199A2EC
#pragma once
//------------------------------------------------------------------------------
#include "abi.hpp"

#if defined( _MSC_VER )
    #include <type_traits>
    #pragma warning( push )
    #pragma warning( disable : 4324 ) // Structure was padded due to __declspec(align())
    namespace std
    {
        template <std::size_t size>
        struct aligned_storage<size, 16>
        { struct __declspec( align( 16 ) ) type { __declspec( align( 16 ) ) char storage[ size ]; }; };
    } // namespace std
    #pragma warning( pop )
#endif
#include <cstddef>
//------------------------------------------------------------------------------
namespace boost { template <class T> class intrusive_ptr; }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

template <class Interface, class BaseInterface>
class PImpl;

template <class Interface>
struct Implementation;

namespace Detail
{
    class PImplTerminator
    {
    protected:
         LE_NOTHROW PImplTerminator() {}
         LE_NOTHROW PImplTerminator( PImplTerminator const & ) {}
    #ifndef NDEBUG
        LE_NOTHROW ~PImplTerminator() {}
    #endif // NDEBUG

    private: // noncopyable
        void operator=( PImplTerminator const & );
    }; // class PImplTerminator

    template <class Interface, class BaseInterface> struct StackPImplBase                             { typedef BaseInterface type; };
    template <class Interface                     > struct StackPImplBase<Interface, PImplTerminator> { typedef PImpl<Interface, PImplTerminator> type; };
} // namespace Detail

template <class Interface, class BaseInterface = Detail::PImplTerminator>
class PImpl : public BaseInterface
{
protected:
    LE_NOTHROWNOALIAS ~PImpl();
}; // class PImpl


template <class Interface, std::size_t storageSize, std::size_t alignment = sizeof( void * ), class BaseInterface = Detail::PImplTerminator>
class StackPImpl
    : public Detail::StackPImplBase<Interface, BaseInterface>::type
{
protected:
    LE_NOTHROWNOALIAS StackPImpl(               );
    LE_NOTHROWNOALIAS StackPImpl( StackPImpl && );
    LE_NOTHROWNOALIAS StackPImpl( Interface  && );
    template <typename T>
    LE_NOTHROWNOALIAS StackPImpl( T          && );

    typedef StackPImpl<Interface, storageSize, alignment, BaseInterface> ConcreteStackPimpl;

private:
    typedef typename Detail::StackPImplBase<Interface, BaseInterface>::type Base;

private:
#ifdef _MSC_VER
    typedef typename std::aligned_storage<storageSize, alignment>::type Placeholder;
#else
    struct Placeholder
    {
        char __attribute__(( __aligned__(( alignment )) )) storage[ storageSize ];
    };
#endif // _MSC_VER
    Placeholder placeholder_;
}; // struct StackPImpl


template <class Interface, class BaseInterface = Detail::PImplTerminator>
class HeapPImpl : public PImpl<Interface, BaseInterface>
{
public:
    typedef boost::intrusive_ptr<Interface      >  Ptr;
    typedef boost::intrusive_ptr<Interface const> CPtr;

    LE_NOTHROWNOALIAS static Ptr  LE_FASTCALL_ABI create();
    LE_NOTHROWNOALIAS static void LE_FASTCALL_ABI operator delete( void * ); ///< \internal

private:
    LE_NOTHROWNOALIAS HeapPImpl(); ///< \internal

private: // boost::intrusive_ptr required section
    friend void LE_NOTHROWNOALIAS LE_FASTCALL_ABI intrusive_ptr_add_ref( Interface const * );
    friend void LE_NOTHROW        LE_FASTCALL_ABI intrusive_ptr_release( Interface const * );
}; // class HeapPImpl

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // pimpl_hpp
