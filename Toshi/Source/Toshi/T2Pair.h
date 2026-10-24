#pragma once
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class First, class Second, class C = TComparator<First>>
struct T2Pair
{
	First  first;
	Second second;

	struct Comparator
	{
		auto operator()( const T2Pair& a, const T2Pair& b ) -> decltype( C()( a.first, b.first ) )
		{
			return C()( a.first, b.first );
		}
	};

	TCONSTEXPR T2Pair()
	    : first( First() )
	    , second( Second() )
	{}

	// $Barnyard: FUNCTION 00581dc0
	// $deBlob: FUNCTION 007e3f90
	TCONSTEXPR T2Pair( const First& a_rcFirst, const Second& a_rcSecond )
	    : first( a_rcFirst )
	    , second( a_rcSecond )
	{}

	TCONSTEXPR T2Pair( const First& a_rcFirst )
	    : first( a_rcFirst )
	    , second( Second() )
	{}

	TCONSTEXPR T2Pair( First&& a_rFirst )
	    : first( a_rFirst )
	    , second( Second() )
	{}

	TCONSTEXPR T2Pair( const First& a_rcFirst, Second&& a_rSecond )
	    : first( a_rcFirst )
	    , second( a_rSecond )
	{}

	TCONSTEXPR T2Pair( First&& a_rFirst, Second&& a_rSecond )
	    : first( a_rFirst )
	    , second( a_rSecond )
	{}

	// $deBlob: FUNCTION 007e3f40
	TCONSTEXPR T2Pair( const T2Pair& a_rcOther )
	    : first( a_rcOther.first )
	    , second( a_rcOther.second )
	{}

	TCONSTEXPR First&  GetFirst() { return first; }
	TCONSTEXPR Second& GetSecond() { return second; }

	TCONSTEXPR T2Pair& operator=( const T2Pair<First, Second, C>& a_rcOther )
	{
		if ( this != &a_rcOther )
		{
			first  = a_rcOther.first;
			second = a_rcOther.second;
		}

		return *this;
	}
};

TOSHI_NAMESPACE_END
