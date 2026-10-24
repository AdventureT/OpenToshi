#pragma once
#include <type_traits>

#define T2_FOREACH( vecName, iteratorName ) \
	for ( auto iteratorName = ( vecName ).Begin(); iteratorName != ( vecName ).End(); ++iteratorName )

#define T2_FOREACH_BACK( vecName, iteratorName ) \
	for ( auto iteratorName = ( vecName ).Tail(); iteratorName != ( vecName ).End(); --iteratorName )

#define T2_FOREACH_ARRAY( arrName, iteratorName ) \
	for ( TINT iteratorName = 0; iteratorName < TARRAYSIZE( arrName ); ++iteratorName )

#define T2_FOREACH_ARRAY_BACK( arrName, iteratorName ) \
	for ( TINT iteratorName = TARRAYSIZE( arrName ) - 1; iteratorName >= 0; --iteratorName )

#define T2_DEFINE_ITERATOR_FRIEND() \
	template <class K, class Node> friend class Toshi::T2Iterator

#define T2_DEFINE_ITERATOR( TYPE, NODE_TYPE ) \
	using Iterator = Toshi::T2Iterator<TYPE, NODE_TYPE>

TOSHI_NAMESPACE_START

template <class K, class Node>
class T2Iterator
{
public:
	TSTATICASSERT( std::is_base_of<Node, K>::value );

	T2Iterator()
	{
		m_pPtr = TNULL;
	}

	T2Iterator( Node* pPtr )
	{
		m_pPtr = pPtr;
	}

	// $Barnyard: FUNCTION 004c6230
	// $Barnyard: FUNCTION 0050d520
	T2Iterator( K* pPtr )
	{
		m_pPtr = pPtr;
	}

	T2Iterator( const T2Iterator& other )
	{
		m_pPtr = other.m_pPtr;
	}

	K* Get()
	{
		return TSTATICCAST( K, m_pPtr );
	}

	const K* Get() const
	{
		return TSTATICCAST( K, m_pPtr );
	}

	K& operator*() TNOEXCEPT
	{
		return *TSTATICCAST( K, m_pPtr );
	}

	const K& operator*() const TNOEXCEPT
	{
		return *TSTATICCAST( K, m_pPtr );
	}

	T2Iterator Next() const
	{
		TASSERT( m_pPtr != TNULL );
		return T2Iterator{ m_pPtr->Next() };
	}

	T2Iterator Prev() const
	{
		TASSERT( m_pPtr != TNULL );
		return T2Iterator{ m_pPtr->Prev() };
	}

	// Compares the pointers of two iterators
	// NOTE: a template constrained to T2Iterator so that it only takes another iterator: a plain operator!=( const
	// T2Iterator& ) makes `it != pNode` ambiguous with the built-in comparison through operator K*()
	// $deBlob: FUNCTION 007de870
	template <class OtherIterator, class = typename std::enable_if<std::is_same<OtherIterator, T2Iterator>::value>::type>
	TBOOL operator!=( const OtherIterator& a_rOther ) const
	{
		return m_pPtr != a_rOther.m_pPtr;
	}

	void operator=( const T2Iterator& other )
	{
		m_pPtr = other.m_pPtr;
	}

	void operator=( K* pPtr )
	{
		m_pPtr = pPtr;
	}

	// $deBlob: FUNCTION 0056f0d0
	// $deBlob: FUNCTION 00571a40
	// $deBlob: FUNCTION 005c4850
	// $deBlob: FUNCTION 005f8de0
	// $deBlob: FUNCTION 005f9860
	// $deBlob: FUNCTION 0060c0c0
	// $deBlob: FUNCTION 0060e340
	// $deBlob: FUNCTION 0060e380
	// $deBlob: FUNCTION 0060e3c0
	// $deBlob: FUNCTION 00610db0
	// $deBlob: FUNCTION 00610df0
	// $deBlob: FUNCTION 00613b20
	// $deBlob: FUNCTION 00613b60
	// $deBlob: FUNCTION 00617970
	// $deBlob: FUNCTION 006179b0
	// $deBlob: FUNCTION 0061d8d0
	// $deBlob: FUNCTION 0061fcb0
	// $deBlob: FUNCTION 0061fcf0
	// $deBlob: FUNCTION 0072f460
	// $deBlob: FUNCTION 0072f4a0
	// $deBlob: FUNCTION 0076a1e0
	// $deBlob: FUNCTION 0076a220
	// $deBlob: FUNCTION 007881e0
	// $deBlob: FUNCTION 0078bd40
	// $deBlob: FUNCTION 0078bd90
	// $deBlob: FUNCTION 0078bde0
	// $deBlob: FUNCTION 0078f6a0
	// $deBlob: FUNCTION 00793aa0
	// $deBlob: FUNCTION 007a45b0
	// $deBlob: FUNCTION 007ceb20
	// $deBlob: FUNCTION 007d0a20
	// $deBlob: FUNCTION 007d12a0
	// $deBlob: FUNCTION 007d41c0
	// $deBlob: FUNCTION 007de8d0
	// $deBlob: FUNCTION 007e0f70
	K* operator->() const
	{
		TASSERT( m_pPtr != TNULL );
		return TSTATICCAST( K, m_pPtr );
	}

	// NOTE: which of 241/248 is operator-> and which the conversion isn't known (same code)
	// $deBlob: FUNCTION 0055db20
	// $deBlob: FUNCTION 0056f050
	// $deBlob: FUNCTION 0056f090
	// $deBlob: FUNCTION 0056feb0
	// $deBlob: FUNCTION 005719f0
	// $deBlob: FUNCTION 0057b000
	// $deBlob: FUNCTION 00591670
	// $deBlob: FUNCTION 00599670
	// $deBlob: FUNCTION 0059ca70
	// $deBlob: FUNCTION 005b64c0
	// $deBlob: FUNCTION 005c0540
	// $deBlob: FUNCTION 005c0580
	// $deBlob: FUNCTION 005c4800
	// $deBlob: FUNCTION 005e9fe0
	// $deBlob: FUNCTION 005f9820
	// $deBlob: FUNCTION 0061fc30
	// $deBlob: FUNCTION 0061fc70
	// $deBlob: FUNCTION 00659990
	// $deBlob: FUNCTION 006b43a0
	// $deBlob: FUNCTION 006be500
	// $deBlob: FUNCTION 006c5150
	// $deBlob: FUNCTION 006dcf30
	// $deBlob: FUNCTION 0072f420
	// $deBlob: FUNCTION 0075b0d0
	// $deBlob: FUNCTION 0075b110
	// $deBlob: FUNCTION 0075b150
	// $deBlob: FUNCTION 00763900
	// $deBlob: FUNCTION 0076a160
	// $deBlob: FUNCTION 0076a1a0
	// $deBlob: FUNCTION 0078bc50
	// $deBlob: FUNCTION 0078bca0
	// $deBlob: FUNCTION 0078bcf0
	// $deBlob: FUNCTION 0078f660
	// $deBlob: FUNCTION 00790610
	// $deBlob: FUNCTION 00791560
	// $deBlob: FUNCTION 00791fc0
	// $deBlob: FUNCTION 00792000
	// $deBlob: FUNCTION 007a4570
	// $deBlob: FUNCTION 007c0700
	// $deBlob: FUNCTION 007ceae0
	// $deBlob: FUNCTION 007de890
	// $deBlob: FUNCTION 007e0f30
	// $deBlob: FUNCTION 007e2650
	operator K*() const
	{
		TASSERT( m_pPtr != TNULL );
		return TSTATICCAST( K, m_pPtr );
	}

	// $deBlob: FUNCTION 0072f530
	// $deBlob: FUNCTION 00790660
	// $Barnyard: FUNCTION 005d6a10
	T2Iterator operator++( TINT )
	{
		TASSERT( m_pPtr != TNULL );
		T2Iterator old = m_pPtr;
		m_pPtr         = m_pPtr->Next();
		return old;
	}

	T2Iterator operator--( TINT )
	{
		TASSERT( m_pPtr != TNULL );
		T2Iterator old = m_pPtr;
		m_pPtr         = m_pPtr->Prev();
		return old;
	}

	// $deBlob: FUNCTION 0055db80
	// $deBlob: FUNCTION 0056f110
	// $deBlob: FUNCTION 0056f160
	// $deBlob: FUNCTION 0056ff00
	// $deBlob: FUNCTION 00571a90
	// $deBlob: FUNCTION 0057b040
	// $deBlob: FUNCTION 0057b090
	// $deBlob: FUNCTION 0057b0e0
	// $deBlob: FUNCTION 005916c0
	// $deBlob: FUNCTION 005996b0
	// $deBlob: FUNCTION 0059cab0
	// $deBlob: FUNCTION 005b6510
	// $deBlob: FUNCTION 005c06c0
	// $deBlob: FUNCTION 005c48a0
	// $deBlob: FUNCTION 005e1ad0
	// $deBlob: FUNCTION 005ea030
	// $deBlob: FUNCTION 005f6c80
	// $deBlob: FUNCTION 005f8e20
	// $deBlob: FUNCTION 005f98a0
	// $deBlob: FUNCTION 005fb8f0
	// $deBlob: FUNCTION 00600800
	// $deBlob: FUNCTION 00604c80
	// $deBlob: FUNCTION 00604cd0
	// $deBlob: FUNCTION 00604d20
	// $deBlob: FUNCTION 0060c100
	// $deBlob: FUNCTION 0060c150
	// $deBlob: FUNCTION 0060e400
	// $deBlob: FUNCTION 0060e450
	// $deBlob: FUNCTION 0060e4a0
	// $deBlob: FUNCTION 00610e30
	// $deBlob: FUNCTION 00610e80
	// $deBlob: FUNCTION 00613ba0
	// $deBlob: FUNCTION 00613bf0
	// $deBlob: FUNCTION 006179f0
	// $deBlob: FUNCTION 00617a40
	// $deBlob: FUNCTION 00617a90
	// $deBlob: FUNCTION 0061ba70
	// $deBlob: FUNCTION 0061d910
	// $deBlob: FUNCTION 0061fd30
	// $deBlob: FUNCTION 0061fd80
	// $deBlob: FUNCTION 006599d0
	// $deBlob: FUNCTION 006b43e0
	// $deBlob: FUNCTION 006be540
	// $deBlob: FUNCTION 006c5190
	// $deBlob: FUNCTION 006dcf80
	// $deBlob: FUNCTION 0072f4e0
	// $deBlob: FUNCTION 0072f580
	// $deBlob: FUNCTION 0075b1a0
	// $deBlob: FUNCTION 0075b1f0
	// $deBlob: FUNCTION 0075b240
	// $deBlob: FUNCTION 00763950
	// $deBlob: FUNCTION 0076a260
	// $deBlob: FUNCTION 0076a2b0
	// $deBlob: FUNCTION 0078be30
	// $deBlob: FUNCTION 0078be80
	// $deBlob: FUNCTION 0078bed0
	// $deBlob: FUNCTION 0078f6e0
	// $deBlob: FUNCTION 007906b0
	// $deBlob: FUNCTION 007915b0
	// $deBlob: FUNCTION 00792040
	// $deBlob: FUNCTION 00792090
	// $deBlob: FUNCTION 007a45f0
	// $deBlob: FUNCTION 007c0740
	// $deBlob: FUNCTION 007ceb60
	// $deBlob: FUNCTION 007d0a60
	// $deBlob: FUNCTION 007d12f0
	// $deBlob: FUNCTION 007de910
	// $deBlob: FUNCTION 007e0fb0
	// $deBlob: FUNCTION 007e1470
	T2Iterator& operator++()
	{
		TASSERT( m_pPtr != TNULL );
		m_pPtr = m_pPtr->Next();
		return *this;
	}

	// $deBlob: FUNCTION 0061fdd0
	// $deBlob: FUNCTION 00625040
	// $deBlob: FUNCTION 006dcfe0
	// $deBlob: FUNCTION 0072f5d0
	// $deBlob: FUNCTION 0072f620
	T2Iterator& operator--()
	{
		TASSERT( m_pPtr != TNULL );
		m_pPtr = m_pPtr->Prev();
		return *this;
	}

private:
	Node* m_pPtr;
};

TOSHI_NAMESPACE_END
