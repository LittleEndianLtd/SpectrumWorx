////////////////////////////////////////////////////////////////////////////////
///
/// \file parentFromMember.hpp
/// --------------------------
///
/// Utility classes for getting an object's parent object (that holds it by
/// value, i.e. it is its member).
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parentFromMember_hpp__185E793C_ECDF_4DFA_9136_5C9E5CA4F353
#define parentFromMember_hpp__185E793C_ECDF_4DFA_9136_5C9E5CA4F353
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic/value_at.hpp>

#include <type_traits>
//------------------------------------------------------------------------------
namespace boost { template <typename T> class optional; }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class DummyStorage
///
/// \brief A helper class creating "empty"/trivially constructed and destructed
/// temporary objects that have the same size and alignment as the class it is
/// trying to impersonate/act as its placeholder (the class specified in the
/// template parameter).
///
////////////////////////////////////////////////////////////////////////////////
 
template <class T>
struct DummyStorage
{
    DummyStorage() {}
    DummyStorage( DummyStorage const & );
    typename std::aligned_storage
    <
        sizeof( T ),
        std::alignment_of<T>::value
    >::type LE_MSVC_SPECIFIC( const ) storage_;//...mrmlj...GCC...
    T const & impersonate() const { return reinterpret_cast<T const &>( storage_ ); }
};


#pragma warning( push )
#pragma warning( disable : 4269 ) // 'const' automatic data initialized with
                                  // compiler generated default constructor
                                  // produces unreliable results

////////////////////////////////////////////////////////////////////////////////
///
/// \class ParentFromMember
///
/// \brief In the following scenario:
///    class B {};
///    class A { B b; }
/// ParentFromMember<A, B, &A::b> retrieves a reference to A given a reference
/// to B (assumes that the reference to B is a reference to a B object that is
/// actually a part of an A object).
///
////////////////////////////////////////////////////////////////////////////////

template
<
    class ParentParam,
    class MemberParam,
    MemberParam ParentParam::* pMember
>
class ParentFromMember
{
public:
    using Parent = ParentParam;
    using Member = MemberParam;

public:
    LE_FORCEINLINE Parent & operator()( Member & member ) const
    {
        char * const pOpaqueMember( reinterpret_cast<char *>( &member ) );
        LE_ASSUME( pOpaqueMember );
        Parent const * const pDummyParent( reinterpret_cast<Parent const *>( pOpaqueMember ) );
        LE_ASSUME( pDummyParent );
        Member const * const pDummyMember( &(pDummyParent->*pMember)                         );
        LE_ASSUME( pDummyMember );
        unsigned int const offset( static_cast<unsigned int>( reinterpret_cast<char const *>( pDummyMember ) - pOpaqueMember ) );
        Parent * const pParent( reinterpret_cast<Parent *>( pOpaqueMember - offset ) );
        LE_ASSUME( pParent );
        return *pParent;
    }

    LE_FORCEINLINE Parent const & operator()( Member const & member ) const { return operator()( const_cast<Member &>( member ) ); }
}; // class ParentFromMember


////////////////////////////////////////////////////////////////////////////////
///
/// \class FusionContainerFromMember
///
/// \brief Retrieves a reference to a FusionContainer given a member index
/// (needed because generally a boost::fusion container need not be unique so
/// an index to a member cannot be deduced from its type) and a reference to a
/// corresponding member (assumes that the member referenced is actually a part
/// of the FusionContainer object).
///
////////////////////////////////////////////////////////////////////////////////

template
<
    class FusionContainerParam,
    unsigned int memberIndex
>
class FusionContainerFromMember
{
public:
    using FusionContainer = FusionContainerParam;
    using Member          = typename boost::fusion::result_of::value_at_c<FusionContainer, memberIndex>::type;

public:
    LE_FORCEINLINE FusionContainer & operator()( Member & member ) const
    {
        DummyStorage<FusionContainer> const fakeContainer;

        ptrdiff_t const offset
        (
            reinterpret_cast<char const *>( &boost::fusion::at_c<memberIndex>( reinterpret_cast<FusionContainer const &>( fakeContainer ) ) )
                -
            reinterpret_cast<char const *>( &fakeContainer                                                                                  )
        );

        return *reinterpret_cast<FusionContainer *>( reinterpret_cast<char *>( &member ) - offset );
    }

    LE_FORCEINLINE FusionContainer const & operator()( Member const & member ) const { return operator()( const_cast<Member &>( member ) ); }
}; // class FusionContainerFromMember


////////////////////////////////////////////////////////////////////////////////
///
/// \class OptionalFromInstance
///
/// \brief In the following scenario:
///    class A {};
///    typedef boost::optional<A> OptionalA;
/// OptionalFromInstance<A> retrieves a reference to OptionalA given a reference
/// to A (assumes that the reference to A is a reference to an A object that is
/// actually stored in a boost::optional<>).
///
////////////////////////////////////////////////////////////////////////////////

template <typename T, bool optionalMustBeInitialised = true>
class OptionalFromInstance
{
public:
    LE_FORCEINLINE boost::optional<T> & operator()( T & instance ) const
    {
        //...mrmlj...assumptions about optional internals...
        // http://www.boost.org/doc/libs/release/libs/optional/doc/html/boost_optional/tutorial/performance_considerations.html
        struct PODOptional { bool is_initialized_flag; std::aligned_storage_t<sizeof( T ), std::alignment_of<T>::value> storage; };
        auto & optionalInstance( *reinterpret_cast<boost::optional<T> *>( ( reinterpret_cast<char *>( &instance ) - offsetof( PODOptional, storage ) ) ) );
        BOOST_ASSERT( !optionalMustBeInitialised || optionalInstance.get_ptr() == &instance );
        return optionalInstance;
    }

    LE_FORCEINLINE boost::optional<T> const & operator()( T const & instance ) const { return operator()( const_cast<T &>( instance ) ); }
}; // OptionalFromInstance


////////////////////////////////////////////////////////////////////////////////
///
/// \class ParentFromOptionalMember
///
/// \brief Same as ParentFromMember but works with objects wrapped/held in
/// boost::optionals.
///
////////////////////////////////////////////////////////////////////////////////

template
<
    class ParentParam,
    class MemberParam,
    boost::optional<MemberParam> ParentParam::* pMember,
    bool optionalMustBeInitialised = true
>
class ParentFromOptionalMember
{
public:
    using Parent = ParentParam;
    using Member = MemberParam;

public:
    LE_FORCEINLINE Parent & operator()( Member & member ) const
    {
        return ParentFromMember<Parent, boost::optional<Member>, pMember>()
               (
                   OptionalFromInstance<Member, optionalMustBeInitialised>()( member )
               );
    }

    LE_FORCEINLINE Parent const & operator()( Member const & member ) const { return operator()( const_cast<Member &>( member ) ); }
}; // class ParentFromOptionalMember


////////////////////////////////////////////////////////////////////////////////
///
/// \class MemberFromMember
///
/// \brief Wraps referencing between member of a (parent) class.
///
////////////////////////////////////////////////////////////////////////////////

template
<
    class ParentParam,
    class SourceMemberParam,
    class TargetMemberParam,
    SourceMemberParam ParentParam::* pSourceMember,
    TargetMemberParam ParentParam::* pTargetMember
>
class MemberFromMember
    :
    private ParentFromMember<ParentParam, SourceMemberParam, pSourceMember>
{
public:
    using Parent       = typename std::remove_const<ParentParam      >::type;
    using SourceMember = typename std::remove_const<SourceMemberParam>::type;
    using TargetMember = typename std::remove_const<TargetMemberParam>::type;

public:
    LE_FORCEINLINE TargetMember & operator()( SourceMember & sourceMember ) const
    {
        Parent & parent( ParentFromMember<Parent, SourceMember, pSourceMember>::operator()( sourceMember ) );
        return parent.*pTargetMember;
    }

    LE_FORCEINLINE TargetMember const & operator()( SourceMember const & sourceMember ) const { return operator()( const_cast<SourceMember &>( sourceMember ) ); }
}; // class MemberFromMember

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parentFromMember_hpp
