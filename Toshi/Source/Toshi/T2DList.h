#pragma once
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// TOSHI 2.0 - Doubly linked list
// Usage example:
//
// class AMyClass : public Toshi::T2DList<AMyClass>::Node { ... };
//
// Toshi::T2DList<AMyClass> list;
// list.PushBack(new AMyClass);
//-----------------------------------------------------------------------------
class T2GenericDList
{
public:
	class Node
	{
	protected:
		friend T2GenericDList;
		template <class T> friend class T2DList;
		T2_DEFINE_ITERATOR_FRIEND();

	public:
		// $deBlob: FUNCTION 0055d8b0
		// $deBlob: FUNCTION 00560c10
		// $deBlob: FUNCTION 0057ad50
		// $deBlob: FUNCTION 0057add0
		// $deBlob: FUNCTION 005f6850
		// $deBlob: FUNCTION 005f9700
		// $deBlob: FUNCTION 006006a0
		// $deBlob: FUNCTION 006138e0
		// $deBlob: FUNCTION 006138f0
		// $deBlob: FUNCTION 0061fab0
		// $deBlob: FUNCTION 006598b0
		// $deBlob: FUNCTION 00769ef0
		// $deBlob: FUNCTION 007a4470
		// $deBlob: FUNCTION 007c05d0
		TCONSTEXPR Node()
		    : m_pNext( this ), m_pPrev( this )
		{}

		// NOTE: de Blob's node doesn't unlink itself, it asserts that it was removed
		// $deBlob: FUNCTION 0055d9a0
		// $deBlob: FUNCTION 0055da10
		// $deBlob: FUNCTION 0055da50
		// $deBlob: FUNCTION 00560cf0
		// $deBlob: FUNCTION 00560d40
		// $deBlob: FUNCTION 00578390
		// $deBlob: FUNCTION 0057ae50
		// $deBlob: FUNCTION 0057aec0
		// $deBlob: FUNCTION 0057af00
		// $deBlob: FUNCTION 0057af40
		// $deBlob: FUNCTION 0057b150
		// $deBlob: FUNCTION 005915d0
		// $deBlob: FUNCTION 005995d0
		// $deBlob: FUNCTION 00599610
		// $deBlob: FUNCTION 00599700
		// $deBlob: FUNCTION 005aa000
		// $deBlob: FUNCTION 005b6450
		// $deBlob: FUNCTION 005d2350
		// $deBlob: FUNCTION 005e15b0
		// $deBlob: FUNCTION 005e9f70
		// $deBlob: FUNCTION 005f6990
		// $deBlob: FUNCTION 005f69d0
		// $deBlob: FUNCTION 005f6a40
		// $deBlob: FUNCTION 005f6a80
		// $deBlob: FUNCTION 005f6ac0
		// $deBlob: FUNCTION 005f7ee0
		// $deBlob: FUNCTION 005f7f20
		// $deBlob: FUNCTION 005f7f60
		// $deBlob: FUNCTION 005f9780
		// $deBlob: FUNCTION 005f97c0
		// $deBlob: FUNCTION 005fb830
		// $deBlob: FUNCTION 005fb870
		// $deBlob: FUNCTION 005fdc00
		// $deBlob: FUNCTION 005fdc40
		// $deBlob: FUNCTION 005fdc80
		// $deBlob: FUNCTION 00600740
		// $deBlob: FUNCTION 00600790
		// $deBlob: FUNCTION 00600850
		// $deBlob: FUNCTION 00604b30
		// $deBlob: FUNCTION 00604bb0
		// $deBlob: FUNCTION 0060bf30
		// $deBlob: FUNCTION 0060bf70
		// $deBlob: FUNCTION 0060bff0
		// $deBlob: FUNCTION 0060c030
		// $deBlob: FUNCTION 0060e0f0
		// $deBlob: FUNCTION 0060e130
		// $deBlob: FUNCTION 0060e1b0
		// $deBlob: FUNCTION 0060e1f0
		// $deBlob: FUNCTION 0060e4f0
		// $deBlob: FUNCTION 0060e580
		// $deBlob: FUNCTION 00610c60
		// $deBlob: FUNCTION 00613990
		// $deBlob: FUNCTION 006139d0
		// $deBlob: FUNCTION 00613a10
		// $deBlob: FUNCTION 00613a50
		// $deBlob: FUNCTION 00613c40
		// $deBlob: FUNCTION 00613cd0
		// $deBlob: FUNCTION 006177a0
		// $deBlob: FUNCTION 00617860
		// $deBlob: FUNCTION 006178a0
		// $deBlob: FUNCTION 00617ae0
		// $deBlob: FUNCTION 00617bd0
		// $deBlob: FUNCTION 0061fb30
		// $deBlob: FUNCTION 0061fb90
		// $deBlob: FUNCTION 0061fe80
		// $deBlob: FUNCTION 006598e0
		// $deBlob: FUNCTION 00659920
		// $deBlob: FUNCTION 006b42e0
		// $deBlob: FUNCTION 006b4320
		// $deBlob: FUNCTION 006be460
		// $deBlob: FUNCTION 006be4a0
		// $deBlob: FUNCTION 006be590
		// $deBlob: FUNCTION 006c5080
		// $deBlob: FUNCTION 006c50c0
		// $deBlob: FUNCTION 006dce00
		// $deBlob: FUNCTION 006dce40
		// $deBlob: FUNCTION 006e92a0
		// $deBlob: FUNCTION 006e9dd0
		// $deBlob: FUNCTION 00735440
		// $deBlob: FUNCTION 00735530
		// $deBlob: FUNCTION 00735620
		// $deBlob: FUNCTION 00735660
		// $deBlob: FUNCTION 0075af50
		// $deBlob: FUNCTION 0075afc0
		// $deBlob: FUNCTION 0075b2c0
		// $deBlob: FUNCTION 0075f1e0
		// $deBlob: FUNCTION 0075f270
		// $deBlob: FUNCTION 00763890
		// $deBlob: FUNCTION 00769fb0
		// $deBlob: FUNCTION 0076a080
		// $deBlob: FUNCTION 0076a500
		// $deBlob: FUNCTION 007881a0
		// $deBlob: FUNCTION 0078bb50
		// $deBlob: FUNCTION 0078bb90
		// $deBlob: FUNCTION 0078f570
		// $deBlob: FUNCTION 0078f5d0
		// $deBlob: FUNCTION 0078f880
		// $deBlob: FUNCTION 007914f0
		// $deBlob: FUNCTION 007a44b0
		// $deBlob: FUNCTION 007a4500
		// $deBlob: FUNCTION 007a4640
		// $deBlob: FUNCTION 007b0be0
		// $deBlob: FUNCTION 007b9e70
		// $deBlob: FUNCTION 007c42f0
		// $deBlob: FUNCTION 007cea30
		// $deBlob: FUNCTION 007cea70
		// $deBlob: FUNCTION 007cebe0
		// $deBlob: FUNCTION 007de7e0
		// $deBlob: FUNCTION 007de830
		// $deBlob: FUNCTION 007de960
		// $deBlob: FUNCTION 007dfdc0
		// $deBlob: FUNCTION 007e25c0
		// $deBlob: FUNCTION 007e2600
		// $deBlob: FUNCTION 007e2720
		// NOTE: retail has a copy of this in every file that destroys a node (the compiler doesn't inline it)
		~Node()
		{
			TASSERT( TFALSE == IsLinkedCall() );
		}

		TBOOL IsLinked() const { return m_pNext != this; }

		// NOTE: retail has the comparison as a real function too (the destructors call it, other users inline the compare)
		TNOINLINE TBOOL IsLinkedCall() const;
		void  InsertBefore( Node* a_pInsertBefore );
		void  InsertAfter( Node* a_pInsertAfter );
		void  Remove();

		Node* Next() const
		{
			return m_pNext;
		}

		Node* Prev() const
		{
			return m_pPrev;
		}

	private:
		Node* m_pNext; // 0x0
		Node* m_pPrev; // 0x4
	};

public:
	T2GenericDList();

	// $deBlob: FUNCTION 0055da90
	// $deBlob: FUNCTION 006252f0
	~T2GenericDList()
	{
		TASSERT( m_oRoot.m_pNext == &m_oRoot );
	}

	TBOOL IsLinked() const
	{
		return m_oRoot.IsLinked();
	}

	// Moves a_pNode (unlinking it from its list) before a_pInsertBefore
	static void Splice( Node* a_pInsertBefore, Node* a_pNode );

	// Moves the nodes a_pFirst...a_pLast (unlinking them from their list) before a_pInsertBefore
	static void Splice( Node* a_pInsertBefore, Node* a_pFirst, Node* a_pLast );

	// Removes the nodes from a_pFirst to a_pLast (excluded)
	static void Erase( Node* a_pFirst, Node* a_pLast );

	void ClearBefore( Node* pNode )
	{
		if ( pNode != &m_oRoot )
			Erase( m_oRoot.m_pNext, pNode );
	}

	void Clear();

	TBOOL IsInList( Node* a_pNode ) const;

	TINT Size();
	TINT Size() const;

protected:
	mutable Node m_oRoot;
};

template <class T>
class T2DList : public T2GenericDList
{
public:
	// The node of a T2DList<T> element
	class Node : public T2GenericDList::Node
	{
	public:
		// NOTE: de Blob emits these forwarders once per T
		// $deBlob: FUNCTION 0055e180
		// $deBlob: FUNCTION 0056d230
		// $deBlob: FUNCTION 0057b650
		// $deBlob: FUNCTION 0057b660
		// $deBlob: FUNCTION 0057b670
		// $deBlob: FUNCTION 005997c0
		// $deBlob: FUNCTION 005f71d0
		// $deBlob: FUNCTION 005f8000
		// $deBlob: FUNCTION 005f9a30
		// $deBlob: FUNCTION 005fba80
		// $deBlob: FUNCTION 005fdd20
		// $deBlob: FUNCTION 00600980
		// $deBlob: FUNCTION 00604e20
		// $deBlob: FUNCTION 00604e30
		// $deBlob: FUNCTION 0060c310
		// $deBlob: FUNCTION 0060c320
		// $deBlob: FUNCTION 0060e720
		// $deBlob: FUNCTION 0060e730
		// $deBlob: FUNCTION 0060e740
		// $deBlob: FUNCTION 006110c0
		// $deBlob: FUNCTION 006110d0
		// $deBlob: FUNCTION 00613e00
		// $deBlob: FUNCTION 00613e10
		// $deBlob: FUNCTION 00617ce0
		// $deBlob: FUNCTION 00617cf0
		// $deBlob: FUNCTION 00617d00
		// $deBlob: FUNCTION 00620070
		// $deBlob: FUNCTION 00620080
		// $deBlob: FUNCTION 00659b10
		// $deBlob: FUNCTION 006b4550
		// $deBlob: FUNCTION 006c5380
		// $deBlob: FUNCTION 006c5390
		// $deBlob: FUNCTION 006cf410
		// $deBlob: FUNCTION 006dd530
		// $deBlob: FUNCTION 00730f80
		// $deBlob: FUNCTION 00730f90
		// $deBlob: FUNCTION 00751c30
		// $deBlob: FUNCTION 0075b520
		// $deBlob: FUNCTION 0075b530
		// $deBlob: FUNCTION 0075c5d0
		// $deBlob: FUNCTION 0076ab90
		// $deBlob: FUNCTION 0076aba0
		// $deBlob: FUNCTION 007882d0
		// $deBlob: FUNCTION 0078fc80
		// $deBlob: FUNCTION 007a47e0
		// $deBlob: FUNCTION 007c08a0
		// $deBlob: FUNCTION 007ced20
		// $deBlob: FUNCTION 007d0d40
		// $deBlob: FUNCTION 007deac0
		// $deBlob: FUNCTION 007e1500
		void InsertBefore( T2GenericDList::Node* a_pInsertBefore ) { T2GenericDList::Node::InsertBefore( a_pInsertBefore ); }

		// $deBlob: FUNCTION 0055e170
		// $deBlob: FUNCTION 00561060
		// $deBlob: FUNCTION 0057b640
		// $deBlob: FUNCTION 005f9a20
		// $deBlob: FUNCTION 00604e00
		// $deBlob: FUNCTION 00604e10
		// $deBlob: FUNCTION 0060e6f0
		// $deBlob: FUNCTION 0060e700
		// $deBlob: FUNCTION 0060e710
		// $deBlob: FUNCTION 00611090
		// $deBlob: FUNCTION 00613df0
		// $deBlob: FUNCTION 00617cd0
		// $deBlob: FUNCTION 00620050
		// $deBlob: FUNCTION 00620060
		// $deBlob: FUNCTION 006b4540
		// $deBlob: FUNCTION 0075b510
		// $deBlob: FUNCTION 007ced10
		void InsertAfter( T2GenericDList::Node* a_pInsertAfter ) { T2GenericDList::Node::InsertAfter( a_pInsertAfter ); }

		// $deBlob: FUNCTION 0055e2d0
		// $deBlob: FUNCTION 00561150
		// $deBlob: FUNCTION 0056d340
		// $deBlob: FUNCTION 005700f0
		// $deBlob: FUNCTION 00571bc0
		// $deBlob: FUNCTION 005786b0
		// $deBlob: FUNCTION 0057b820
		// $deBlob: FUNCTION 0057b830
		// $deBlob: FUNCTION 00591950
		// $deBlob: FUNCTION 00599930
		// $deBlob: FUNCTION 005b67a0
		// $deBlob: FUNCTION 005c4ab0
		// $deBlob: FUNCTION 005e1c80
		// $deBlob: FUNCTION 005ea360
		// $deBlob: FUNCTION 005f7480
		// $deBlob: FUNCTION 005f7490
		// $deBlob: FUNCTION 005f80a0
		// $deBlob: FUNCTION 005f9ac0
		// $deBlob: FUNCTION 005fbb40
		// $deBlob: FUNCTION 005fddb0
		// $deBlob: FUNCTION 00600a10
		// $deBlob: FUNCTION 00604f40
		// $deBlob: FUNCTION 00604f50
		// $deBlob: FUNCTION 0060c500
		// $deBlob: FUNCTION 0060c510
		// $deBlob: FUNCTION 0060e8d0
		// $deBlob: FUNCTION 0060e8e0
		// $deBlob: FUNCTION 0060e8f0
		// $deBlob: FUNCTION 00611220
		// $deBlob: FUNCTION 00611230
		// $deBlob: FUNCTION 00613f10
		// $deBlob: FUNCTION 00613f20
		// $deBlob: FUNCTION 00617f30
		// $deBlob: FUNCTION 00617f40
		// $deBlob: FUNCTION 00617f50
		// $deBlob: FUNCTION 00620200
		// $deBlob: FUNCTION 00620210
		// $deBlob: FUNCTION 006256b0
		// $deBlob: FUNCTION 00659b90
		// $deBlob: FUNCTION 006b45e0
		// $deBlob: FUNCTION 006be7b0
		// $deBlob: FUNCTION 006dd570
		// $deBlob: FUNCTION 007312e0
		// $deBlob: FUNCTION 007312f0
		// $deBlob: FUNCTION 0075b6f0
		// $deBlob: FUNCTION 0075b700
		// $deBlob: FUNCTION 0075b710
		// $deBlob: FUNCTION 0075b720
		// $deBlob: FUNCTION 0075c630
		// $deBlob: FUNCTION 00763c10
		// $deBlob: FUNCTION 0076b060
		// $deBlob: FUNCTION 0076b070
		// $deBlob: FUNCTION 00788420
		// $deBlob: FUNCTION 0078c6a0
		// $deBlob: FUNCTION 0078c6b0
		// $deBlob: FUNCTION 0078c6c0
		// $deBlob: FUNCTION 0078dc80
		// $deBlob: FUNCTION 00790010
		// $deBlob: FUNCTION 00790840
		// $deBlob: FUNCTION 00791790
		// $deBlob: FUNCTION 007a4870
		// $deBlob: FUNCTION 007b0ca0
		// $deBlob: FUNCTION 007c0930
		// $deBlob: FUNCTION 007cedc0
		// $deBlob: FUNCTION 007d0ec0
		// $deBlob: FUNCTION 007d1530
		// $deBlob: FUNCTION 007deb60
		// $deBlob: FUNCTION 007e1520
		void Remove() { T2GenericDList::Node::Remove(); }
	};

	T2_DEFINE_ITERATOR( T, T2GenericDList::Node );

public:
	// $deBlob: FUNCTION 00591550
	// $deBlob: FUNCTION 0055d850
	// $deBlob: FUNCTION 00560bb0
	// $deBlob: FUNCTION 0056cfa0
	// $deBlob: FUNCTION 0056fdf0
	// $deBlob: FUNCTION 005718e0
	// $deBlob: FUNCTION 0057acd0
	// $deBlob: FUNCTION 0057ace0
	// $deBlob: FUNCTION 0057acf0
	// $deBlob: FUNCTION 0059c530
	// $deBlob: FUNCTION 005b6310
	// $deBlob: FUNCTION 005c01d0
	// $deBlob: FUNCTION 005c01e0
	// $deBlob: FUNCTION 005c0210
	// $deBlob: FUNCTION 005c4740
	// $deBlob: FUNCTION 005e1a70
	// $deBlob: FUNCTION 005e9dd0
	// $deBlob: FUNCTION 005f67a0
	// $deBlob: FUNCTION 005f67b0
	// $deBlob: FUNCTION 005f67c0
	// $deBlob: FUNCTION 005f7eb0
	// $deBlob: FUNCTION 005f9690
	// $deBlob: FUNCTION 005fb6d0
	// $deBlob: FUNCTION 005fdbd0
	// $deBlob: FUNCTION 00600640
	// $deBlob: FUNCTION 006049c0
	// $deBlob: FUNCTION 006049d0
	// $deBlob: FUNCTION 0060bd60
	// $deBlob: FUNCTION 0060bd70
	// $deBlob: FUNCTION 0060df70
	// $deBlob: FUNCTION 0060df80
	// $deBlob: FUNCTION 0060df90
	// $deBlob: FUNCTION 00610a90
	// $deBlob: FUNCTION 00610aa0
	// $deBlob: FUNCTION 006137c0
	// $deBlob: FUNCTION 006137d0
	// $deBlob: FUNCTION 00617620
	// $deBlob: FUNCTION 00617630
	// $deBlob: FUNCTION 00617640
	// $deBlob: FUNCTION 0061fa90
	// $deBlob: FUNCTION 0061faa0
	// $deBlob: FUNCTION 00624dd0
	// $deBlob: FUNCTION 00659790
	// $deBlob: FUNCTION 006b4280
	// $deBlob: FUNCTION 006be380
	// $deBlob: FUNCTION 006c4fe0
	// $deBlob: FUNCTION 006c4ff0
	// $deBlob: FUNCTION 006dcbb0
	// $deBlob: FUNCTION 0072d200
	// $deBlob: FUNCTION 0072d210
	// $deBlob: FUNCTION 00751100
	// $deBlob: FUNCTION 0075add0
	// $deBlob: FUNCTION 0075ade0
	// $deBlob: FUNCTION 0075adf0
	// $deBlob: FUNCTION 0075c490
	// $deBlob: FUNCTION 00763720
	// $deBlob: FUNCTION 00769df0
	// $deBlob: FUNCTION 00769e00
	// $deBlob: FUNCTION 00769e10
	// $deBlob: FUNCTION 00769e20
	// $deBlob: FUNCTION 00769e30
	// $deBlob: FUNCTION 0078b8e0
	// $deBlob: FUNCTION 0078b8f0
	// $deBlob: FUNCTION 0078b900
	// $deBlob: FUNCTION 0078b910
	// $deBlob: FUNCTION 0078f3d0
	// $deBlob: FUNCTION 0078f3e0
	// $deBlob: FUNCTION 007904e0
	// $deBlob: FUNCTION 007913a0
	// $deBlob: FUNCTION 007a4460
	// $deBlob: FUNCTION 007c05a0
	// $deBlob: FUNCTION 007ce980
	// $deBlob: FUNCTION 007d0970
	// $deBlob: FUNCTION 007d0980
	// $deBlob: FUNCTION 007d1240
	// $deBlob: FUNCTION 007de740
	// $deBlob: FUNCTION 007dfd90
	// $Barnyard: FUNCTION 00516030
	T2DList()
	{
		TSTATICASSERT( std::is_base_of<T2GenericDList::Node, T>::value );
	}

	// NOTE: the empty check is T2GenericDList's
	// $deBlob: FUNCTION 00591710
	// $deBlob: FUNCTION 0055d950
	// $deBlob: FUNCTION 00560ca0
	// $deBlob: FUNCTION 0056d000
	// $deBlob: FUNCTION 0056fe30
	// $deBlob: FUNCTION 00571960
	// $deBlob: FUNCTION 0057ade0
	// $deBlob: FUNCTION 0057adf0
	// $deBlob: FUNCTION 0057ae00
	// $deBlob: FUNCTION 005915b0
	// $deBlob: FUNCTION 005995a0
	// $deBlob: FUNCTION 005b63b0
	// $deBlob: FUNCTION 005c0350
	// $deBlob: FUNCTION 005c0360
	// $deBlob: FUNCTION 005c0400
	// $deBlob: FUNCTION 005c4780
	// $deBlob: FUNCTION 005e1a90
	// $deBlob: FUNCTION 005e9eb0
	// $deBlob: FUNCTION 005f6920
	// $deBlob: FUNCTION 005f6930
	// $deBlob: FUNCTION 005f6940
	// $deBlob: FUNCTION 005f7ed0
	// $deBlob: FUNCTION 005fb7f0
	// $deBlob: FUNCTION 005fdbf0
	// $deBlob: FUNCTION 006006e0
	// $deBlob: FUNCTION 00604a30
	// $deBlob: FUNCTION 00604a40
	// $deBlob: FUNCTION 0060bed0
	// $deBlob: FUNCTION 0060bee0
	// $deBlob: FUNCTION 0060e050
	// $deBlob: FUNCTION 0060e060
	// $deBlob: FUNCTION 0060e070
	// $deBlob: FUNCTION 00610ba0
	// $deBlob: FUNCTION 00610bb0
	// $deBlob: FUNCTION 00613900
	// $deBlob: FUNCTION 00613910
	// $deBlob: FUNCTION 00617700
	// $deBlob: FUNCTION 00617710
	// $deBlob: FUNCTION 00617720
	// $deBlob: FUNCTION 0061fb10
	// $deBlob: FUNCTION 0061fb20
	// $deBlob: FUNCTION 00624ed0
	// $deBlob: FUNCTION 006598c0
	// $deBlob: FUNCTION 006be400
	// $deBlob: FUNCTION 0072e1b0
	// $deBlob: FUNCTION 0072e1c0
	// $deBlob: FUNCTION 007512b0
	// $deBlob: FUNCTION 0075aed0
	// $deBlob: FUNCTION 0075aee0
	// $deBlob: FUNCTION 0075aef0
	// $deBlob: FUNCTION 0075c4f0
	// $deBlob: FUNCTION 007637f0
	// $deBlob: FUNCTION 00769f10
	// $deBlob: FUNCTION 00769f20
	// $deBlob: FUNCTION 00769f30
	// $deBlob: FUNCTION 00769f40
	// $deBlob: FUNCTION 0078ba20
	// $deBlob: FUNCTION 0078ba30
	// $deBlob: FUNCTION 0078ba40
	// $deBlob: FUNCTION 0078ba50
	// $deBlob: FUNCTION 0078f500
	// $deBlob: FUNCTION 0078f510
	// $deBlob: FUNCTION 00790560
	// $deBlob: FUNCTION 00791450
	// $deBlob: FUNCTION 007a44a0
	// $deBlob: FUNCTION 007c05e0
	// $deBlob: FUNCTION 007ce9d0
	// $deBlob: FUNCTION 007d09c0
	// $deBlob: FUNCTION 007d09d0
	// $deBlob: FUNCTION 007d1260
	// $deBlob: FUNCTION 007de7b0
	// $deBlob: FUNCTION 007dfdb0
	~T2DList() = default;

	Iterator Head() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.Next();
	}

	Iterator Tail() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.Prev();
	}

	// $deBlob: FUNCTION 0055dd40
	// $deBlob: FUNCTION 0056ff80
	// $deBlob: FUNCTION 00571ae0
	// $deBlob: FUNCTION 0057b250
	// $deBlob: FUNCTION 0057b260
	// $deBlob: FUNCTION 0057b270
	// $deBlob: FUNCTION 005917d0
	// $deBlob: FUNCTION 00599770
	// $deBlob: FUNCTION 0059cec0
	// $deBlob: FUNCTION 005b65c0
	// $deBlob: FUNCTION 005c07d0
	// $deBlob: FUNCTION 005c4920
	// $deBlob: FUNCTION 005e1b50
	// $deBlob: FUNCTION 005ea1c0
	// $deBlob: FUNCTION 005f6f40
	// $deBlob: FUNCTION 005f8e70
	// $deBlob: FUNCTION 005f98f0
	// $deBlob: FUNCTION 005fba00
	// $deBlob: FUNCTION 00600950
	// $deBlob: FUNCTION 00604d70
	// $deBlob: FUNCTION 00604d80
	// $deBlob: FUNCTION 00604d90
	// $deBlob: FUNCTION 0060c1d0
	// $deBlob: FUNCTION 0060c1e0
	// $deBlob: FUNCTION 0060e670
	// $deBlob: FUNCTION 0060e680
	// $deBlob: FUNCTION 0060e690
	// $deBlob: FUNCTION 00610fd0
	// $deBlob: FUNCTION 00610fe0
	// $deBlob: FUNCTION 00613d90
	// $deBlob: FUNCTION 00613da0
	// $deBlob: FUNCTION 00617c60
	// $deBlob: FUNCTION 00617c70
	// $deBlob: FUNCTION 00617c80
	// $deBlob: FUNCTION 0061ff10
	// $deBlob: FUNCTION 0061ff20
	// $deBlob: FUNCTION 00659a90
	// $deBlob: FUNCTION 0075b370
	// $deBlob: FUNCTION 0075b380
	// $deBlob: FUNCTION 0075b390
	// $deBlob: FUNCTION 00763a80
	// $deBlob: FUNCTION 0076a670
	// $deBlob: FUNCTION 0076a680
	// $deBlob: FUNCTION 0078c010
	// $deBlob: FUNCTION 0078c020
	// $deBlob: FUNCTION 0078c030
	// $deBlob: FUNCTION 0078f8f0
	// $deBlob: FUNCTION 00790700
	// $deBlob: FUNCTION 00791690
	// $deBlob: FUNCTION 007a4780
	// $deBlob: FUNCTION 007c0870
	// $deBlob: FUNCTION 007cec40
	// $deBlob: FUNCTION 007d0b80
	// $deBlob: FUNCTION 007d1340
	// $deBlob: FUNCTION 007d4340
	// $deBlob: FUNCTION 007dea90
	// $deBlob: FUNCTION 007e14c0
	// $deBlob: FUNCTION 00560e50
	// $deBlob: FUNCTION 006b4500
	// $deBlob: FUNCTION 006be6c0
	// $deBlob: FUNCTION 006c5320
	// $deBlob: FUNCTION 006dd230
	// $Barnyard: FUNCTION 005e94e0
	Iterator Begin() const
	{
		return m_oRoot.Next();
	}

	// $deBlob: FUNCTION 0056ff90
	// $deBlob: FUNCTION 00571af0
	// $deBlob: FUNCTION 0057b2b0
	// $deBlob: FUNCTION 0057b2c0
	// $deBlob: FUNCTION 0057b2d0
	// $deBlob: FUNCTION 005917f0
	// $deBlob: FUNCTION 00599790
	// $deBlob: FUNCTION 0059cf10
	// $deBlob: FUNCTION 005b6640
	// $deBlob: FUNCTION 005c0820
	// $deBlob: FUNCTION 005c0830
	// $deBlob: FUNCTION 005c49a0
	// $deBlob: FUNCTION 005e1b70
	// $deBlob: FUNCTION 005ea1e0
	// $deBlob: FUNCTION 005f6f90
	// $deBlob: FUNCTION 005f8e80
	// $deBlob: FUNCTION 005f9910
	// $deBlob: FUNCTION 005fba20
	// $deBlob: FUNCTION 00600970
	// $deBlob: FUNCTION 00604dc0
	// $deBlob: FUNCTION 00604dd0
	// $deBlob: FUNCTION 00604de0
	// $deBlob: FUNCTION 0060c220
	// $deBlob: FUNCTION 0060c230
	// $deBlob: FUNCTION 0060e6a0
	// $deBlob: FUNCTION 0060e6b0
	// $deBlob: FUNCTION 0060e6c0
	// $deBlob: FUNCTION 00611010
	// $deBlob: FUNCTION 00611020
	// $deBlob: FUNCTION 00613dd0
	// $deBlob: FUNCTION 00613de0
	// $deBlob: FUNCTION 00617c90
	// $deBlob: FUNCTION 00617ca0
	// $deBlob: FUNCTION 00617cb0
	// $deBlob: FUNCTION 0061ff70
	// $deBlob: FUNCTION 0061ff80
	// $deBlob: FUNCTION 00659ab0
	// $deBlob: FUNCTION 0075b440
	// $deBlob: FUNCTION 0075b450
	// $deBlob: FUNCTION 0075b460
	// $deBlob: FUNCTION 00763ac0
	// $deBlob: FUNCTION 0076a760
	// $deBlob: FUNCTION 0076a770
	// $deBlob: FUNCTION 0078c190
	// $deBlob: FUNCTION 0078c1a0
	// $deBlob: FUNCTION 0078c1b0
	// $deBlob: FUNCTION 0078fa00
	// $deBlob: FUNCTION 00790720
	// $deBlob: FUNCTION 007916b0
	// $deBlob: FUNCTION 007a47a0
	// $deBlob: FUNCTION 007c0880
	// $deBlob: FUNCTION 007cecf0
	// $deBlob: FUNCTION 007d0ba0
	// $deBlob: FUNCTION 007d13c0
	// $deBlob: FUNCTION 007deab0
	// $deBlob: FUNCTION 00560ec0
	// $deBlob: FUNCTION 006b4520
	// $deBlob: FUNCTION 006be6d0
	// $deBlob: FUNCTION 006c5350
	// $deBlob: FUNCTION 006dd270
	// $Barnyard: FUNCTION 00623810
	Iterator End() const
	{
		return &m_oRoot;
	}

	Iterator RBegin() const
	{
		return m_oRoot.Next();
	}

	Iterator REnd() const
	{
		return &m_oRoot;
	}

	// $deBlob: FUNCTION 005ea1f0
	// $deBlob: FUNCTION 0060c2b0
	// $deBlob: FUNCTION 0061ff90
	// $deBlob: FUNCTION 0061ffe0
	// $deBlob: FUNCTION 00659ac0
	// $deBlob: FUNCTION 0076a7c0
	// $deBlob: FUNCTION 0076a800
	// $deBlob: FUNCTION 0079f3d0
	// $deBlob: FUNCTION 007d0bb0
	T* Front() const
	{
		TASSERT( !IsEmpty() );
		return TSTATICCAST( T, m_oRoot.Next() );
	}

	// $deBlob: FUNCTION 005ea170
	// $deBlob: FUNCTION 00600910
	// $deBlob: FUNCTION 00659a50
	// $deBlob: FUNCTION 0075b320
	// $deBlob: FUNCTION 00763a30
	// $deBlob: FUNCTION 0076a5f0
	// $deBlob: FUNCTION 0076a630
	// $deBlob: FUNCTION 007d0b40
	// $deBlob: FUNCTION 007d42f0
	T* Back() const
	{
		TASSERT( !IsEmpty() );
		return TSTATICCAST( T, m_oRoot.Prev() );
	}

	void Delete( Iterator iter )
	{
		iter->Remove();
		delete TSTATICCAST( T, iter );
	}

	// $deBlob: FUNCTION 0056d180
	// $deBlob: FUNCTION 005b65d0
	// $deBlob: FUNCTION 0075b3a0
	// $deBlob: FUNCTION 0078c0b0
	// $deBlob: FUNCTION 0078c120
	// $deBlob: FUNCTION 0078db90
	// $deBlob: FUNCTION 0078f960
	// $deBlob: FUNCTION 007cec50
	// $deBlob: FUNCTION 007d1350
	// $deBlob: FUNCTION 0078a740
	// $deBlob: FUNCTION 005c4930
	// $Barnyard: FUNCTION 0057c4d0
	// $Barnyard: FUNCTION 0057d520
	void DeleteAll()
	{
		while ( !IsEmpty() )
		{
			delete PopFront();
		}
	}

	// $deBlob: FUNCTION 0057b2e0
	// $deBlob: FUNCTION 005b6650
	// $deBlob: FUNCTION 0060c240
	// $deBlob: FUNCTION 006dd290
	// $deBlob: FUNCTION 0075b470
	// $deBlob: FUNCTION 00790730
	// $Barnyard: FUNCTION 00457a20
	Iterator Erase( Iterator iter )
	{
		Iterator itNext = iter.Next();
		TSTATICCAST( T2GenericDList::Node, iter.Get() )->Remove(); // T can have other bases with a Remove method

		return itNext;
	}

	// $Barnyard: FUNCTION 006c8140
	TBOOL Exists( T* a_pItem )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it == a_pItem )
				return TTRUE;
		}

		return TFALSE;
	}

	// $deBlob: FUNCTION 0055e1d0
	// $deBlob: FUNCTION 00561080
	// $deBlob: FUNCTION 0056d240
	// $deBlob: FUNCTION 00571b60
	// $deBlob: FUNCTION 00571b70
	// $deBlob: FUNCTION 0057b680
	// $deBlob: FUNCTION 0057b690
	// $deBlob: FUNCTION 0057b6a0
	// $deBlob: FUNCTION 00591880
	// $deBlob: FUNCTION 00599800
	// $deBlob: FUNCTION 005b66f0
	// $deBlob: FUNCTION 005c49e0
	// $deBlob: FUNCTION 005e1bd0
	// $deBlob: FUNCTION 005ea2a0
	// $deBlob: FUNCTION 005f8020
	// $deBlob: FUNCTION 005f9a40
	// $deBlob: FUNCTION 005fba90
	// $deBlob: FUNCTION 005fdd30
	// $deBlob: FUNCTION 00600990
	// $deBlob: FUNCTION 00604e40
	// $deBlob: FUNCTION 00604e50
	// $deBlob: FUNCTION 0060c330
	// $deBlob: FUNCTION 0060c340
	// $deBlob: FUNCTION 0060e750
	// $deBlob: FUNCTION 0060e760
	// $deBlob: FUNCTION 0060e770
	// $deBlob: FUNCTION 00611130
	// $deBlob: FUNCTION 00611140
	// $deBlob: FUNCTION 00613e20
	// $deBlob: FUNCTION 00613e30
	// $deBlob: FUNCTION 00617dd0
	// $deBlob: FUNCTION 00617de0
	// $deBlob: FUNCTION 00617df0
	// $deBlob: FUNCTION 0061bb10
	// $deBlob: FUNCTION 0061da30
	// $deBlob: FUNCTION 006200a0
	// $deBlob: FUNCTION 006200b0
	// $deBlob: FUNCTION 006200c0
	// $deBlob: FUNCTION 00659b20
	// $deBlob: FUNCTION 006b4560
	// $deBlob: FUNCTION 006be750
	// $deBlob: FUNCTION 00730fa0
	// $deBlob: FUNCTION 0075b570
	// $deBlob: FUNCTION 0075b580
	// $deBlob: FUNCTION 0075b590
	// $deBlob: FUNCTION 0075b5a0
	// $deBlob: FUNCTION 0075c5e0
	// $deBlob: FUNCTION 00763b50
	// $deBlob: FUNCTION 00763b60
	// $deBlob: FUNCTION 0076abb0
	// $deBlob: FUNCTION 0076abc0
	// $deBlob: FUNCTION 0078c2b0
	// $deBlob: FUNCTION 0078c2c0
	// $deBlob: FUNCTION 0078c2d0
	// $deBlob: FUNCTION 0078dc10
	// $deBlob: FUNCTION 0078fd90
	// $deBlob: FUNCTION 007907f0
	// $deBlob: FUNCTION 007921c0
	// $deBlob: FUNCTION 007a47f0
	// $deBlob: FUNCTION 007c08b0
	// $deBlob: FUNCTION 007ced30
	// $deBlob: FUNCTION 007d0d50
	// $deBlob: FUNCTION 007d1430
	// $deBlob: FUNCTION 007dead0
	// $deBlob: FUNCTION 006dd540
	// $deBlob: FUNCTION 006c53a0
	TBOOL IsEmpty() const
	{
		return !m_oRoot.IsLinked();
	}

	// $deBlob: FUNCTION 0056d370
	// $deBlob: FUNCTION 0060c570
	// $deBlob: FUNCTION 0060c580
	// $deBlob: FUNCTION 007cede0
	// $deBlob: FUNCTION 006dd580
	// $deBlob: FUNCTION 006c53e0
	// $deBlob: FUNCTION 00659ba0
	TINT Size() { return T2GenericDList::Size(); }
	TINT Size() const { return T2GenericDList::Size(); }

	// $deBlob: FUNCTION 005fbaa0
	// $deBlob: FUNCTION 005fdd40
	// $deBlob: FUNCTION 0076abd0
	// $deBlob: FUNCTION 007c08c0
	// $deBlob: FUNCTION 0075c5f0
	// $deBlob: FUNCTION 0075b5c0
	// $deBlob: FUNCTION 0075b5b0
	// $deBlob: FUNCTION 005f8030
	TBOOL IsInList( T2GenericDList::Node* a_pNode ) const { return T2GenericDList::IsInList( a_pNode ); }

	// $deBlob: FUNCTION 006c5330
	// $deBlob: FUNCTION 006c5340
	// $deBlob: FUNCTION 007514f0
	// $deBlob: FUNCTION 0055dd50
	// $deBlob: FUNCTION 00560e60
	// $deBlob: FUNCTION 0057b280
	// $deBlob: FUNCTION 0057b290
	// $deBlob: FUNCTION 0057b2a0
	// $deBlob: FUNCTION 005917e0
	// $deBlob: FUNCTION 005e1b60
	// $deBlob: FUNCTION 005ea1d0
	// $deBlob: FUNCTION 005f6f80
	// $deBlob: FUNCTION 005f7ff0
	// $deBlob: FUNCTION 005f9900
	// $deBlob: FUNCTION 005fba10
	// $deBlob: FUNCTION 005fdd10
	// $deBlob: FUNCTION 00600960
	// $deBlob: FUNCTION 00604da0
	// $deBlob: FUNCTION 00604db0
	// $deBlob: FUNCTION 0060c1f0
	// $deBlob: FUNCTION 0060c200
	// $deBlob: FUNCTION 00610ff0
	// $deBlob: FUNCTION 00611000
	// $deBlob: FUNCTION 00613db0
	// $deBlob: FUNCTION 00613dc0
	// $deBlob: FUNCTION 0061ff30
	// $deBlob: FUNCTION 0061ff40
	// $deBlob: FUNCTION 00659aa0
	// $deBlob: FUNCTION 006b4510
	// $deBlob: FUNCTION 006dd260
	// $deBlob: FUNCTION 0075c5a0
	// $deBlob: FUNCTION 00763ab0
	// $deBlob: FUNCTION 0076a690
	// $deBlob: FUNCTION 0076a6a0
	// $deBlob: FUNCTION 0078c040
	// $deBlob: FUNCTION 00790710
	// $deBlob: FUNCTION 007916a0
	// $deBlob: FUNCTION 007a4790
	// $deBlob: FUNCTION 007deaa0
	void Clear()
	{
		T2GenericDList::Clear();
	}

	// Inserts a_pItem before a_itWhere and returns an iterator to it
	// NOTE: Barnyard folds every instantiation; de Blob has one per instantiation (identified by their callers)
	// $deBlob: FUNCTION 006110a0
	// $deBlob: FUNCTION 0076ab50
	// $deBlob: FUNCTION 0076ab70
	// $deBlob: FUNCTION 007882b0
	// $deBlob: FUNCTION 0078fbb0
	// $deBlob: FUNCTION 007d0d20
	// $deBlob: FUNCTION 007e14e0
	// $Barnyard: FUNCTION 005d6a20
	Iterator InsertBefore( Iterator a_itWhere, T* a_pItem )
	{
		a_pItem->InsertBefore( a_itWhere );
		return a_pItem;
	}

	// Inserts a_pItem after a_itWhere and returns an iterator to it
	// $deBlob: FUNCTION 0078c1e0
	// $deBlob: FUNCTION 007d4430
	// $deBlob: FUNCTION 007e6ba0
	Iterator InsertAfter( Iterator a_itWhere, T* a_pItem )
	{
		a_pItem->InsertAfter( a_itWhere );
		return a_pItem;
	}

	// Moves every node of a_rFromList before a_itInsertBefore
	// $deBlob: FUNCTION 005ea370
	// $deBlob: FUNCTION 00763c20
	// $deBlob: FUNCTION 0076b080
	// $deBlob: FUNCTION 0076b160
	// $deBlob: FUNCTION 007a4960
	// $deBlob: FUNCTION 007c0940
	void Splice( Iterator a_itInsertBefore, T2DList& a_rFromList )
	{
		TASSERT( &a_rFromList != this );
		TASSERT( !a_rFromList.IsEmpty(), "Don't splice from an empty list or the Root node will be moved!" );

		T2GenericDList::Node* pLast = a_rFromList.End()->Prev();
		T2GenericDList::Splice( a_itInsertBefore, a_rFromList.Begin(), pLast );
	}

	// $deBlob: FUNCTION 00570070
	// $deBlob: FUNCTION 00571b80
	// $deBlob: FUNCTION 00591910
	// $deBlob: FUNCTION 005b6760
	// $deBlob: FUNCTION 005c4a70
	// $deBlob: FUNCTION 005e1c40
	// $deBlob: FUNCTION 005f7410
	// $deBlob: FUNCTION 006c53b0
	// $deBlob: FUNCTION 006c53c0
	// $deBlob: FUNCTION 006cf430
	// $deBlob: FUNCTION 006dd560
	// $deBlob: FUNCTION 00751f00
	// $deBlob: FUNCTION 0075b6a0
	// $deBlob: FUNCTION 00788410
	// $deBlob: FUNCTION 0078c5e0
	// $deBlob: FUNCTION 0078c620
	// $deBlob: FUNCTION 0078c660
	// $deBlob: FUNCTION 00791750
	// $deBlob: FUNCTION 007d14b0
	// $deBlob: FUNCTION 0055e2b0
	// $deBlob: FUNCTION 0056d330
	// $deBlob: FUNCTION 0057b7b0
	// $deBlob: FUNCTION 0057b7c0
	// $deBlob: FUNCTION 0057b7d0
	// $deBlob: FUNCTION 00599920
	// $deBlob: FUNCTION 005f8090
	// $deBlob: FUNCTION 005f9aa0
	// $deBlob: FUNCTION 005fbb30
	// $deBlob: FUNCTION 005fdda0
	// $deBlob: FUNCTION 00600a00
	// $deBlob: FUNCTION 00604f00
	// $deBlob: FUNCTION 00604f10
	// $deBlob: FUNCTION 0060c4e0
	// $deBlob: FUNCTION 0060c4f0
	// $deBlob: FUNCTION 0060e870
	// $deBlob: FUNCTION 0060e880
	// $deBlob: FUNCTION 0060e890
	// $deBlob: FUNCTION 006111f0
	// $deBlob: FUNCTION 00611200
	// $deBlob: FUNCTION 00613ee0
	// $deBlob: FUNCTION 00613ef0
	// $deBlob: FUNCTION 00617ef0
	// $deBlob: FUNCTION 00617f00
	// $deBlob: FUNCTION 00617f10
	// $deBlob: FUNCTION 006201c0
	// $deBlob: FUNCTION 006201d0
	// $deBlob: FUNCTION 00659b80
	// $deBlob: FUNCTION 006b45c0
	// $deBlob: FUNCTION 00731170
	// $deBlob: FUNCTION 00731180
	// $deBlob: FUNCTION 0075b680
	// $deBlob: FUNCTION 0075b690
	// $deBlob: FUNCTION 0075c620
	// $deBlob: FUNCTION 0076b050
	// $deBlob: FUNCTION 00790000
	// $deBlob: FUNCTION 007a4860
	// $deBlob: FUNCTION 007c0920
	// $deBlob: FUNCTION 007ceda0
	// $deBlob: FUNCTION 007deb50
	// $deBlob: FUNCTION 007e6c70
	// $Barnyard: FUNCTION 004c6260
	void PushBack( T* pItem )
	{
		pItem->InsertBefore( &m_oRoot );
	}

	// $deBlob: FUNCTION 0055e2c0
	// $deBlob: FUNCTION 00561140
	// $deBlob: FUNCTION 005700b0
	// $deBlob: FUNCTION 0057b7e0
	// $deBlob: FUNCTION 005f9ab0
	// $deBlob: FUNCTION 00604f20
	// $deBlob: FUNCTION 00604f30
	// $deBlob: FUNCTION 0060e8a0
	// $deBlob: FUNCTION 0060e8b0
	// $deBlob: FUNCTION 0060e8c0
	// $deBlob: FUNCTION 00611210
	// $deBlob: FUNCTION 00613f00
	// $deBlob: FUNCTION 00617f20
	// $deBlob: FUNCTION 006201e0
	// $deBlob: FUNCTION 006201f0
	// $deBlob: FUNCTION 006b45d0
	// $deBlob: FUNCTION 0075b6e0
	// $deBlob: FUNCTION 007cedb0
	// $deBlob: FUNCTION 007d14f0
	void PushFront( T* pItem )
	{
		pItem->InsertAfter( &m_oRoot );
	}

	// $deBlob: FUNCTION 005610f0
	// $deBlob: FUNCTION 0056d290
	// $deBlob: FUNCTION 0057b6c0
	// $deBlob: FUNCTION 005918b0
	// $deBlob: FUNCTION 005998d0
	// $deBlob: FUNCTION 005ea2c0
	// $deBlob: FUNCTION 006009b0
	// $deBlob: FUNCTION 006200d0
	// $deBlob: FUNCTION 00620120
	// $deBlob: FUNCTION 006be760
	// $deBlob: FUNCTION 00763b70
	// $deBlob: FUNCTION 0076ac90
	// $deBlob: FUNCTION 0076ace0
	// $deBlob: FUNCTION 0078c310
	// $deBlob: FUNCTION 0078c370
	// $deBlob: FUNCTION 0078fe20
	// $deBlob: FUNCTION 007916f0
	// $deBlob: FUNCTION 007a4810
	// $deBlob: FUNCTION 007d0dd0
	// $deBlob: FUNCTION 007ddc00
	// $deBlob: FUNCTION 007deb00
	// $deBlob: FUNCTION 007e6c10
	T* PopBack()
	{
		TASSERT( !IsEmpty() );
		T* pItem = TSTATICCAST( T, m_oRoot.Prev() );
		TSTATICCAST( T2GenericDList::Node, pItem )->Remove();
		return pItem;
	}

	// $deBlob: FUNCTION 0055e260
	// $deBlob: FUNCTION 0056d2e0
	// $deBlob: FUNCTION 0057b710
	// $deBlob: FUNCTION 0057b760
	// $deBlob: FUNCTION 005b6700
	// $deBlob: FUNCTION 005c4a10
	// $deBlob: FUNCTION 005e1be0
	// $deBlob: FUNCTION 005f8040
	// $deBlob: FUNCTION 005f9a50
	// $deBlob: FUNCTION 005fbae0
	// $deBlob: FUNCTION 005fdd50
	// $deBlob: FUNCTION 00604e60
	// $deBlob: FUNCTION 00604eb0
	// $deBlob: FUNCTION 0060c440
	// $deBlob: FUNCTION 0060c490
	// $deBlob: FUNCTION 0060e780
	// $deBlob: FUNCTION 0060e7d0
	// $deBlob: FUNCTION 0060e820
	// $deBlob: FUNCTION 00611150
	// $deBlob: FUNCTION 006111a0
	// $deBlob: FUNCTION 00613e40
	// $deBlob: FUNCTION 00613e90
	// $deBlob: FUNCTION 00617e00
	// $deBlob: FUNCTION 00617e50
	// $deBlob: FUNCTION 00617ea0
	// $deBlob: FUNCTION 00620170
	// $deBlob: FUNCTION 00659b30
	// $deBlob: FUNCTION 006b4570
	// $deBlob: FUNCTION 007310d0
	// $deBlob: FUNCTION 0075b610
	// $deBlob: FUNCTION 0076ad30
	// $deBlob: FUNCTION 0076ad80
	// $deBlob: FUNCTION 0078c3d0
	// $deBlob: FUNCTION 0078c430
	// $deBlob: FUNCTION 0078c490
	// $deBlob: FUNCTION 0078dc20
	// $deBlob: FUNCTION 0078fed0
	// $deBlob: FUNCTION 007c08d0
	// $deBlob: FUNCTION 007ced50
	// $deBlob: FUNCTION 007d1450
	T* PopFront()
	{
		TASSERT( !IsEmpty() );
		T* pItem = TSTATICCAST( T, m_oRoot.Next() );
		TSTATICCAST( T2GenericDList::Node, pItem )->Remove();
		return pItem;
	}
};

//-----------------------------------------------------------------------------
// Purpose: wraps typename T making it T2DList::Node
//-----------------------------------------------------------------------------
template <typename T>
class T2DListNodeWrapper : public T2DList<T2DListNodeWrapper<T>>::Node
{
public:
	using Type = T;

public:
	// constructors/destructor
	TCONSTEXPR T2DListNodeWrapper() = default;

	TCONSTEXPR T2DListNodeWrapper( T* a_pValue )
	    : m_pValue( a_pValue ) {}

	~T2DListNodeWrapper() = default;

	TCONSTEXPR T*       GetNodeValue() { return m_pValue; }
	TCONSTEXPR const T* GetNodeValue() const { return m_pValue; }
	TCONSTEXPR T*       operator->() { return m_pValue; }
	TCONSTEXPR const T* operator->() const { return m_pValue; }

	TCONSTEXPR void SetNodeValue( T* a_pValue ) { m_pValue = a_pValue; }

	TCONSTEXPR T2DListNodeWrapper& operator=( T* a_pValue )
	{
		SetNodeValue( a_pValue );
		return *this;
	}

private:
	T* m_pValue;
};

TOSHI_NAMESPACE_END
