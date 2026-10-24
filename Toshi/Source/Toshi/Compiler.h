#pragma once
//-----------------------------------------------------------------------------
// Compiler.h: lets the sources compile with VS2026 (C++20, the normal build) and
// with VS2013 Update 5 (cl 18.00, toolset v120: retail's compiler, used by the
// matching build in DevTools/Matching). Included first by Typedefs.h.
//
// Use these instead of the C++11/14/17/20 keywords VS2013 lacks:
//   TCONSTEXPR        constexpr functions and constructors (plain inline on VS2013,
//                     so their results can't be used where a constant is required)
//   TCONSTEXPR_VAR    constexpr variables (const on VS2013: integral ones still
//                     work as constants, in-class ones must be integral)
//   TCONSTINIT        constinit (nothing on VS2013)
//   TNOEXCEPT         noexcept (throw() on VS2013)
//   TALIGNAS( N )     alignas( N ) (__declspec( align( N ) ) on VS2013)
//   TALIGNOF( T )     alignof( T ) (__alignof( T ) on VS2013)
//   TALIGNOF_ABSTRACT( T ) alignof( T ) of a class that may be abstract (VS2013 can't: the
//                     pointer size there)
//   TNOINLINE         __declspec( noinline ) (functions retail keeps out of line although tiny)
//   TNODISCARD, TMAYBE_UNUSED, TFALLTHROUGH, TLIKELY, TUNLIKELY, TDEPRECATED,
//   TDEPRECATED_REASON( "..." ): [[attributes]]
// and the helpers below instead of the newer std:: pieces (exchange, bit_cast,
// bit_width). Other rules for VS2013: no inline variables (define static members
// out of line), no if constexpr / concepts / fold expressions (overloads, SFINAE,
// static_assert), no auto return deduction, no in-class array member initialisers,
// no variable templates (std::is_same_v...).
//-----------------------------------------------------------------------------

#if defined( _MSC_VER ) && _MSC_VER < 1900
#  define TOSHI_COMPILER_VS2013
#endif

#ifdef TOSHI_COMPILER_VS2013

#  define TCONSTEXPR     inline
#  define TCONSTEXPR_VAR const
#  define TCONSTINIT
#  define TNOEXCEPT      throw()
#  define TALIGNAS( N )  __declspec( align( N ) )
#  define TALIGNOF( T )  __alignof( T )
#  define TALIGNOF_ABSTRACT( T ) 1 // NOTE: VS2013 can't take __alignof of an abstract class; retail stores 1
#  define TNOINLINE      __declspec( noinline )
#  define TNODISCARD
#  define TMAYBE_UNUSED
#  define TFALLTHROUGH
#  define TLIKELY
#  define TUNLIKELY
#  define TDEPRECATED                  __declspec( deprecated )
#  define TDEPRECATED_REASON( REASON ) __declspec( deprecated( REASON ) )

#else // TOSHI_COMPILER_VS2013

#  define TCONSTEXPR     constexpr
#  define TCONSTEXPR_VAR constexpr
#  define TCONSTINIT     constinit
#  define TNOEXCEPT      noexcept
#  define TALIGNAS( N )  alignas( N )
#  define TALIGNOF( T )  alignof( T )
#  define TALIGNOF_ABSTRACT( T ) 1 // NOTE: retail stores 1 for an abstract class (never created)
#  define TNOINLINE      __declspec( noinline )
#  define TNODISCARD     [[nodiscard]]
#  define TMAYBE_UNUSED  [[maybe_unused]]
#  define TFALLTHROUGH   [[fallthrough]]
#  define TLIKELY        [[likely]]
#  define TUNLIKELY      [[unlikely]]
#  define TDEPRECATED                  [[deprecated]]
#  define TDEPRECATED_REASON( REASON ) [[deprecated( REASON )]]

#endif // !TOSHI_COMPILER_VS2013

#include <string.h>
#include <utility>
#ifndef TOSHI_COMPILER_VS2013
#  include <bit>
#endif

namespace Toshi
{

// std::exchange (C++14)
template <class T, class U>
inline T TExchange( T& a_rObject, U&& a_rNewValue )
{
	T oldValue = std::move( a_rObject );
	a_rObject  = std::forward<U>( a_rNewValue );
	return oldValue;
}

// std::bit_cast (C++20), not usable in constant expressions
template <class To, class From>
inline To TBitCast( const From& a_rFrom )
{
	static_assert( sizeof( To ) == sizeof( From ), "TBitCast: sizes differ" );
	To to;
	memcpy( &to, &a_rFrom, sizeof( To ) );
	return to;
}

// std::bit_width (C++20): number of bits needed to represent the value
template <class T>
TCONSTEXPR int TBitWidth( T a_uiValue )
{
#ifdef TOSHI_COMPILER_VS2013
	int iWidth = 0;
	for ( ; a_uiValue != 0; a_uiValue >>= 1 ) iWidth++;
	return iWidth;
#else
	return std::bit_width( a_uiValue );
#endif
}

} // namespace Toshi
