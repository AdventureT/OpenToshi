/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

HK_FORCE_INLINE int hkBitField::getNumWords() const
{
	return m_words.getSize();
}

HK_FORCE_INLINE hkUint32 hkBitField::getWord( int wordIndex ) const
{
	HK_ASSERT( 0x38a87b3b, wordIndex >= 0 && wordIndex < getNumWords() );

	return m_words[wordIndex];
}

HK_FORCE_INLINE void hkBitField::assignAll( int value )
{
	HK_ASSERT( 0xa59289bb, value >= 0 && value <= 1 );

	hkUint32 fill = value ? 0xffffffff : 0;

	for( int i = 0; i < getNumWords(); i++ )
	{
		m_words[i] = fill;
	}
}

HK_FORCE_INLINE hkBitField::hkBitField()
:	m_words(0),
	m_numBits(0)
{
}

HK_FORCE_INLINE hkBitField::hkBitField( int numBits )
:	m_words( ( numBits + 31 ) >> 5 ),
	m_numBits(numBits)
{
}

HK_FORCE_INLINE hkBitField::hkBitField( int numBits, int initialValue )
:	m_words( ( numBits + 31 ) >> 5 ),
	m_numBits(numBits)
{
	HK_ASSERT( 0xa63ab345, initialValue >= 0 && initialValue <= 1 );
	assignAll( initialValue );
}

HK_FORCE_INLINE hkBitField::hkBitField( hkUint32* ptr, int numBits )
:   m_words( ptr, ( numBits + 31 ) >> 5, ( numBits + 31 ) >> 5 ),
	m_numBits(numBits)
{
}

inline hkBitField::~hkBitField()
{
}

HK_FORCE_INLINE int hkBitField::getSize() const
{
	return m_numBits;
}

HK_FORCE_INLINE int hkBitField::get( int index ) const
{
	HK_ASSERT( 0x48d17bd3, index >= 0 && index < getSize() );

	int arrayIndex = index >> 5;
	return ( ( m_words[arrayIndex] >> ( index & 0x1f ) ) & 1 );
}

HK_FORCE_INLINE void hkBitField::set( int index )
{
	HK_ASSERT( 0x48a97bc3, index >= 0 && index < getSize() );

	int arrayIndex = index >> 5;
	m_words[arrayIndex] |= ( 1 << ( index & 0x1f ) );
}

HK_FORCE_INLINE void hkBitField::clear( int index )
{
	HK_ASSERT( 0x38a87bb3, index >= 0 && index < getSize() );

	int arrayIndex = index >> 5;
	m_words[arrayIndex] &= ~( 1 << ( index & 0x1f ) );
}

HK_FORCE_INLINE void hkBitField::assign( int index, int value )
{
	HK_ASSERT( 0x48a27b13, index >= 0 && index < getSize() );
	HK_ASSERT( 0xe68bb345, value >= 0 && value <= 1 );

	int arrayIndex = index >> 5;
	hkUint32 mask = 1 << (index & 0x1f);

	m_words[arrayIndex] = ( m_words[arrayIndex] & ~mask ) | ( mask & ~( value - 1 ) );
}

HK_FORCE_INLINE void hkBitField::operator = ( const hkBitField& bitField )
{
	m_words = bitField.m_words;
	m_numBits = bitField.m_numBits;
}

HK_FORCE_INLINE hkBitField::hkBitField( hkFinishLoadedObjectFlag flag ) : m_words(flag)
{
}

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
