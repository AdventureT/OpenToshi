#pragma once
#include "Toshi/Defines.h"
#include "Toshi/T2Iterator.h"

TOSHI_NAMESPACE_START

template <TUINT NUM_BITS>
class TBitField
{
public:
	using ValueType                             = TUINT32;
	static TCONSTEXPR_VAR TUINT MIN_NUM_BITS    = 8 * sizeof( ValueType );
	static TCONSTEXPR_VAR TUINT BIT_SUBNUM_MASK = MIN_NUM_BITS - 1;

public:
	TBitField()
	{
		T2_FOREACH_ARRAY( m_Values, i )
		{
			m_Values[ i ] = 0;
		}
	}

	// $deBlob: FUNCTION 005e3dc0
	// $deBlob: FUNCTION 006f9590
	// $Barnyard: FUNCTION 00413220
	void Set( TINT a_iBitNum, TBOOL a_bSet )
	{
		TASSERT( a_iBitNum >= 0 );
		TASSERT( a_iBitNum < NUM_BITS );

		TUINT uiBit = 1 << ( a_iBitNum & BIT_SUBNUM_MASK );

		if ( a_bSet ) m_Values[ a_iBitNum / MIN_NUM_BITS ] |= uiBit;
		else
			m_Values[ a_iBitNum / MIN_NUM_BITS ] &= ~uiBit;
	}

	// $deBlob: FUNCTION 0058c840
	// $deBlob: FUNCTION 006f92d0
	// $Barnyard: FUNCTION 0040e940
	TBOOL IsSet( TINT a_iBitNum ) const
	{
		TASSERT( a_iBitNum >= 0 );
		TASSERT( a_iBitNum < NUM_BITS );

		return ( m_Values[ a_iBitNum / MIN_NUM_BITS ] & ( 1 << ( a_iBitNum & BIT_SUBNUM_MASK ) ) ) != 0;
	}

	// Clears every bit
	void Clear()
	{
		T2_FOREACH_ARRAY( m_Values, i )
		{
			m_Values[ i ] = 0;
		}
	}

	// NOTE: TBitField<32> instance used on AUnitFlagSet masks by AInteraction::UpdatePriority
	// $Barnyard: FUNCTION 0059e630
	TBOOL IsEmpty() const
	{
		T2_FOREACH_ARRAY( m_Values, i )
		{
			if ( m_Values[ i ] != 0 )
				return TFALSE;
		}

		return TTRUE;
	}

private:
	ValueType m_Values[ ( NUM_BITS + BIT_SUBNUM_MASK ) / MIN_NUM_BITS ];
};

TOSHI_NAMESPACE_END
