#include "ToshiPCH.h"
#include "TRandom.h"

#ifdef USE_PORTABLE
#  define cut( a ) ( (a)&0xffffffff ) /* Cut the integer down to 32bits */
#else
#  define cut( a ) ( a ) /* A no-op */
#endif

#define ind( mm, x ) ( ( mm )[ ( x >> 2 ) & ( RANDSIZ - 1 ) ] )
/* the call to cut() is a macro defined in standard.h */
#define rngstep( mix, a, b, mm, m, m2, r, x )               \
	{                                                       \
		x        = *m;                                      \
		a        = cut( ( a ^ ( mix ) ) + *( m2++ ) );      \
		*( m++ ) = y = cut( ind( mm, x ) + a + b );         \
		*( r++ ) = b = cut( ind( mm, y >> RANDSIZL ) + x ); \
	}

#define mix( a, b, c, d, e, f, g, h ) \
	{                                 \
		a ^= b << 11;                 \
		d += a;                       \
		b += c;                       \
		b ^= c >> 2;                  \
		e += b;                       \
		c += d;                       \
		c ^= d << 8;                  \
		f += c;                       \
		d += e;                       \
		d ^= e >> 16;                 \
		g += d;                       \
		e += f;                       \
		e ^= f << 10;                 \
		h += e;                       \
		f += g;                       \
		f ^= g >> 4;                  \
		a += f;                       \
		g += h;                       \
		g ^= h << 8;                  \
		b += g;                       \
		h += a;                       \
		h ^= a >> 9;                  \
		c += h;                       \
		a += b;                       \
	}

#define shuffle( a, b, mm, m, m2, r, x )           \
	{                                              \
		rngstep( a << 13, a, b, mm, m, m2, r, x ); \
		rngstep( a >> 6, a, b, mm, m, m2, r, x );  \
		rngstep( a << 2, a, b, mm, m, m2, r, x );  \
		rngstep( a >> 16, a, b, mm, m, m2, r, x ); \
	}

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00673d70
// $Barnyard: FUNCTION 006b5ca0
TRandom::TRandom()
{
	THPTimer timer = THPTimer();

	LARGE_INTEGER liCounter;
	QueryPerformanceCounter( &liCounter );

	m_uiSeed = liCounter.LowPart;
	RandInit();
}

// $deBlob: FUNCTION 00673e00
// $Barnyard: FUNCTION 006b5850
void TRandom::Isaac()
{
	TUINT a, b, x, y, *m, *mm, *m2, *r, *mend;
	mm = m_pRandmem;
	r  = m_pRandrsl;
	a  = m_uiRandA;
	b  = cut( m_uiRandB + ( ++m_uiRandC ) );

	m    = mm;
	mend = m2 = m + ( RANDSIZ / 2 );
	while ( m < mend )
	{
		shuffle( a, b, mm, m, m2, r, x );
	}

	m2 = mm;
	while ( m2 < mend )
	{
		shuffle( a, b, mm, m, m2, r, x );
	}

	m_uiRandA = a;
	m_uiRandB = b;
}

// $deBlob: FUNCTION 006740a0
// $Barnyard: FUNCTION 006b5af0
void TRandom::RandInit()
{
	TUINT32 a, b, c, d, e, f, g, h;
	TINT    i;

	m_uiRandA = m_uiRandB = m_uiRandC = 0;

	/* Initialize a to h with the golden ratio and seed */
	a = b = c = d = e = f = g = h = 0x9e3779b9 + m_uiSeed;

	/* scramble it */
	for ( i = 0; i < RANDSIZL; i++ )
	{
		mix( a, b, c, d, e, f, g, h );
	}

	// NOTE: the original is randinit() without a seed array (flag FALSE): one pass filling m[] from the mixed seed,
	// m_pRandrsl is not used
	TUINT32* m = m_pRandmem;

	for ( i = 0; i < RANDSIZ; i += 8 )
	{
		mix( a, b, c, d, e, f, g, h );

		m[ i ]     = a;
		m[ i + 1 ] = b;
		m[ i + 2 ] = c;
		m[ i + 3 ] = d;
		m[ i + 4 ] = e;
		m[ i + 5 ] = f;
		m[ i + 6 ] = g;
		m[ i + 7 ] = h;
	}

	Isaac();
	m_uiRndCnt = RANDSIZ;
}

// $deBlob: FUNCTION 0057b470
TINT TRandom::GetInt( TUINT32 a_iLower, TUINT32 a_iUpper )
{
	TASSERT( a_iUpper > a_iLower );
	return a_iLower + GetInt( a_iUpper - a_iLower );
}

// $deBlob: FUNCTION 00673db0
// $Barnyard: FUNCTION 006b5ce0
TINT TRandom::GetInt( TUINT32 a_iUpper )
{
	// NOTE: unlike GetInt( 0, a_iUpper ) there's no assert
	return TINT( ( TUINT64( a_iUpper ) * RandRaw() ) >> 32 );
}

TINT TRandom::GetInt()
{
	return RandRaw();
}

// $deBlob: FUNCTION 0057b420
// $Barnyard: FUNCTION 00401030
TFLOAT TRandom::GetFloat()
{
	return ( RandRaw() >> 1 ) * 4.6566129E-10f;
}

// $deBlob: FUNCTION 00590160
// $Barnyard: FUNCTION 00401080
TFLOAT TRandom::GetFloatMinMax( TFLOAT a_fLower, TFLOAT a_fUpper )
{
	return (TFLOAT)( RandRaw() >> 1 ) * 4.6566129E-10f * ( a_fUpper - a_fLower ) + a_fLower;
}

// Returns a random number from -1 to 1
// $deBlob: FUNCTION 00718450
// $Barnyard: FUNCTION 00608c90
TFLOAT TRandom::GetFloat2()
{
	return ( RandRaw() >> 1 ) * 9.3132257E-10f - 1.0f;
}

// $deBlob: FUNCTION 006743c0
void TRandom::SetSeed( TUINT a_uiSeed )
{
	m_uiSeed = a_uiSeed;
	RandInit();
}

// $deBlob: FUNCTION 0079a3d0
// $Barnyard: FUNCTION 00608c60
TUINT32 TRandom::RandRaw()
{
	if ( ( m_uiRndCnt-- ) != 0 )
	{
		return m_pRandrsl[ m_uiRndCnt ];
	}

	Isaac();
	m_uiRndCnt = RANDSIZ - 1;
	return m_pRandrsl[ RANDSIZ - 1 ];
}

TOSHI_NAMESPACE_END
