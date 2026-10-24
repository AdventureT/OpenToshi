#pragma once
#include "T2Allocator.h"

#undef GetObject

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericObjectPool : protected T2Allocator
{
protected:
	struct UnusedObject
	{
		UnusedObject* pNextObject;
	};

public:
	// NOTE: retail has IsAddressInPool at +0x20 and the destructor at +0x24
	virtual TBOOL IsAddressInPool( const void* a_pAddress ) const = 0;       // +0x20
	virtual ~T2GenericObjectPool()                                = default; // +0x24

protected:
	virtual void* Malloc( TSIZE a_uiSize, TSIZE a_uiAlignment ) OVERRIDE
	{
		return TNULL;
	}

	virtual void* Malloc( TSIZE a_uiSize ) OVERRIDE
	{
		return GetObject();
	}

	virtual void Free( void* a_pPtr ) OVERRIDE
	{
		ReturnObject( TSTATICCAST( UnusedObject, a_pPtr ) );
	}

	virtual TBOOL CanAllocate( TSIZE size ) const OVERRIDE
	{
		return m_pHead != TNULL;
	}

	virtual void* TryMalloc( TSIZE size, TSIZE alignment ) OVERRIDE
	{
		return TNULL;
	}

	// $Barnyard: FUNCTION 00573870
	virtual void* TryMalloc( TSIZE size ) OVERRIDE
	{
		if ( m_pHead != TNULL )
		{
			return GetObject();
		}

		return TNULL;
	}

protected:
	// $deBlob: FUNCTION 00831540
	// $Barnyard: FUNCTION 006cdae0
	void Initialise( UnusedObject* a_pObjects, TUINT a_uiMaxNumber, TUINT a_uiObjectSize )
	{
		m_pHead               = a_pObjects;
		UnusedObject* pObject = a_pObjects;

		for ( TUINT i = a_uiMaxNumber - 1; i != 0; i-- )
		{
			UnusedObject* pNext  = TREINTERPRETCAST( UnusedObject*, TREINTERPRETCAST( TUINTPTR, pObject ) + a_uiObjectSize );
			pObject->pNextObject = pNext;
			pObject              = pNext;
		}

		pObject->pNextObject = TNULL;
	}

	// $deBlob: FUNCTION 008315a0
	UnusedObject* GetObject()
	{
		TASSERT( m_pHead != TNULL );
		UnusedObject* pNode = m_pHead;
		m_pHead             = m_pHead->pNextObject;
		return pNode;
	}

	// $deBlob: FUNCTION 008315e0
	// $Barnyard: FUNCTION 006cdb50
	void ReturnObject( UnusedObject* a_pObject )
	{
		a_pObject->pNextObject = m_pHead;
		m_pHead                = a_pObject;
	}

	// $deBlob: FUNCTION 00831580
	// $Barnyard: FUNCTION 006cdb20
	TUINT GetNumFreeObjects() const
	{
		TUINT uiNumber = 0;
		for ( auto it = m_pHead; it != TNULL; it = it->pNextObject, uiNumber++ )
			;

		return uiNumber;
	}

protected:
	UnusedObject* m_pHead;
};

template <class T, TUINT MaxNumber, TUINT ObjectSize = sizeof( T ), TUINT Alignment = TALIGNOF( T )>
class T2ObjectPool : protected T2GenericObjectPool
{
public:
	TSTATICASSERT( MaxNumber >= 1 );
	TSTATICASSERT( sizeof( T ) >= sizeof( T2GenericObjectPool::UnusedObject ) );

public:
	// $deBlob: FUNCTION 007e2510
	T2ObjectPool()
	{
		T2GenericObjectPool::Initialise(
		    TREINTERPRETCAST( T2GenericObjectPool::UnusedObject*, GetObjects() ),
		    MaxNumber,
		    ObjectSize
		);
	}

	// $Barnyard: FUNCTION 005cc050
	// $Barnyard: FUNCTION 005cc0b0
	// $Barnyard: FUNCTION 0045c5e0
	// $Barnyard: FUNCTION 00555340
	// $deBlob: FUNCTION 007e2690
	// $deBlob: FUNCTION 007e2580
	virtual ~T2ObjectPool()
	{
		TASSERT( GetNumFreeObjects() == MaxNumber );
	}

	// de Blob's T2ObjectPool (T2ObjectPool<AAIBehaviour::Enemy, 4, 32, 4>) checks the sizes and addresses
	// $deBlob: FUNCTION 007e28c0
	virtual void* Malloc( TSIZE a_uiSize ) OVERRIDE
	{
		TASSERT( a_uiSize <= ObjectSize );
		return GetObject();
	}

	// $deBlob: FUNCTION 007e2950
	virtual void* TryMalloc( TSIZE a_uiSize ) OVERRIDE
	{
		TASSERT( a_uiSize <= ObjectSize );
		return ( m_pHead != TNULL ) ? GetObject() : TNULL;
	}

	// $deBlob: FUNCTION 007e27c0
	virtual void Free( void* a_pData ) OVERRIDE
	{
		TASSERT( a_pData >= &m_aObjects[ 0 ] && a_pData < &m_aObjects[ MaxNumber * ObjectSize ] );
		TASSERT( ( (ptrdiff_t)a_pData - (ptrdiff_t)m_aObjects ) % ObjectSize == 0 );
		ReturnObject( TSTATICCAST( UnusedObject, a_pData ) );
	}

	// $Barnyard: FUNCTION 005cc0d0
	// $Barnyard: FUNCTION 0045ca20
	// $Barnyard: FUNCTION 00555230
	template <class... Args>
	T* NewObject( Args&&... args )
	{
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( ObjectSize ) );
		T* pValue = new ( Malloc( ObjectSize ) ) T( std::forward<Args>( args )... );
		return pValue;
	}

	T* AllocateObject()
	{
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( ObjectSize ) );
		return TSTATICCAST( T, Malloc( ObjectSize ) );
	}

	void FreeObject( T* a_pObject )
	{
		TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
		Free( a_pObject );
	}

	// NOTE: de Blob's has no IsAddressInPool assert
	// $deBlob: FUNCTION 007e2480
	void DeleteObject( T* a_pObject )
	{
		a_pObject->~T();
		Free( a_pObject );
	}

	// NOTE: de Blob's T2GenericObjectPool has only pure slots (vftable of __purecall), every T2ObjectPool<>
	// instantiation has its own copy of these
	using T2GenericObjectPool::Malloc;
	using T2GenericObjectPool::TryMalloc;

	// $deBlob: FUNCTION 007e2910
	virtual void* Malloc( TSIZE a_uiSize, TSIZE a_uiAlignment ) OVERRIDE { return TNULL; } // +0x00

	// $deBlob: FUNCTION 007e2790
	virtual TBOOL CanAllocate( TSIZE a_uiSize ) const OVERRIDE { return T2GenericObjectPool::CanAllocate( a_uiSize ); } // +0x0C

	// $deBlob: FUNCTION 007e29b0
	virtual void* TryMalloc( TSIZE a_uiSize, TSIZE a_uiAlignment ) OVERRIDE { return TNULL; } // +0x10

	// $Barnyard: FUNCTION 005cc060
	// $deBlob: FUNCTION 007e2930
	// $Barnyard: FUNCTION 0045c590
	// $Barnyard: FUNCTION 005552e0
	virtual TSIZE GetUsedSize() const OVERRIDE
	{
		return ( MaxNumber - GetNumFreeObjects() ) * ObjectSize;
	}

	// $deBlob: FUNCTION 007e27b0
	// $Barnyard: FUNCTION 0045c5b0
	// $Barnyard: FUNCTION 00555300
	virtual TSIZE GetCapacity() const OVERRIDE
	{
		return MaxNumber * ObjectSize;
	}

	// $deBlob: FUNCTION 007e2890
	// $Barnyard: FUNCTION 0045c5c0
	// $Barnyard: FUNCTION 005cc090
	// $Barnyard: FUNCTION 00555320
	virtual TBOOL IsAddressInPool( const void* a_pAddress ) const
	{
		return ( TREINTERPRETCAST( TUINTPTR, this ) + sizeof( T2GenericObjectPool ) ) <= TREINTERPRETCAST( TUINTPTR, a_pAddress ) &&
		    TREINTERPRETCAST( TUINTPTR, a_pAddress ) < ( TREINTERPRETCAST( TUINTPTR, this ) + sizeof( T2GenericObjectPool ) + ( MaxNumber * ObjectSize ) );
	}

	// $Barnyard: FUNCTION 00555310
	TBOOL CanAllocate() const { return T2GenericObjectPool::CanAllocate( ObjectSize ); }

private:
	T* GetObjects()
	{
		return TREINTERPRETCAST( T*, m_aObjects );
	}

	const T* GetObjects() const
	{
		return TREINTERPRETCAST( const T*, m_aObjects );
	}

private:
	TBYTE m_aObjects[ MaxNumber * ObjectSize ];
};

template <class T>
class T2DynamicObjectPool : protected T2GenericObjectPool
{
public:
	T2DynamicObjectPool( T2Allocator* a_pAllocator, TINT a_iMaxNumber, TSIZE a_uiSize = sizeof( T ), TSIZE a_uiAlignment = TALIGNOF( T ) )
	{
		TVALIDPTR( a_pAllocator );
		m_iMaxNumber = a_iMaxNumber;
		m_pAllocator = a_pAllocator;

		TUINT32 uiClassSize      = a_uiSize;
		TUINT32 uiClassAlignment = a_uiAlignment;
		m_uiObjectSize           = ( uiClassSize - 1 ) + uiClassAlignment & ~( uiClassAlignment - 1U );

		TASSERT( m_uiObjectSize >= a_uiSize && uiClassAlignment > 0 );

		if ( m_iMaxNumber < 1 )
		{
			m_pData = TNULL;
		}
		else
		{
			m_pData = m_pAllocator->Malloc( m_iMaxNumber * m_uiObjectSize, uiClassAlignment );
		}

		TVALIDPTR( m_pData );
		T2GenericObjectPool::Initialise(
		    TREINTERPRETCAST( T2GenericObjectPool::UnusedObject*, m_pData ),
		    m_iMaxNumber,
		    m_uiObjectSize
		);
	}

	// $Barnyard: FUNCTION 005d63b0
	// $Barnyard: FUNCTION 005d63e0
	// $Barnyard: FUNCTION 005d6410
	virtual ~T2DynamicObjectPool()
	{
		m_pAllocator->Free( m_pData );
	}

	// $Barnyard: FUNCTION 005d6a50
	// $Barnyard: FUNCTION 005d6a70
	// $Barnyard: FUNCTION 005d6aa0
	// $Barnyard: FUNCTION 005d6ae0
	template <class... Args>
	T* NewObject( Args&&... args )
	{
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
		T* pValue = new ( Malloc( m_uiObjectSize ) ) T( std::forward<Args>( args )... );
		return pValue;
	}

	T* AllocateObject()
	{
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
		return TSTATICCAST( T, Malloc( m_uiObjectSize ) );
	}

	void FreeObject( T* a_pObject )
	{
		TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
		Free( a_pObject );
	}

	void DeleteObject( T* a_pObject )
	{
		TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
		a_pObject->~T();
		Free( a_pObject );
	}

	virtual TSIZE GetUsedSize() const OVERRIDE
	{
		return ( m_iMaxNumber - GetNumFreeObjects() ) * m_uiObjectSize;
	}

	virtual TSIZE GetCapacity() const OVERRIDE
	{
		return m_iMaxNumber * m_uiObjectSize;
	}

	virtual TBOOL IsAddressInPool( const void* a_pAddress ) const
	{
		return TREINTERPRETCAST( TUINTPTR, m_pData ) <= TREINTERPRETCAST( TUINTPTR, a_pAddress ) &&
		    TREINTERPRETCAST( TUINTPTR, a_pAddress ) < TREINTERPRETCAST( TUINTPTR, m_pData ) + ( m_iMaxNumber * m_uiObjectSize );
	}

	TBOOL CanAllocate() const { return T2GenericObjectPool::CanAllocate( m_uiObjectSize ); }

private:
	T2Allocator* m_pAllocator;
	TINT         m_iMaxNumber;
	TUINT        m_uiObjectSize;
	void*        m_pData;
};

template <class TClassType>
class T2DerivedDynamicObjectPool : protected T2GenericObjectPool
{
public:
	// $Barnyard: FUNCTION 00572200
	// $Barnyard: FUNCTION 00573960
	// $Barnyard: FUNCTION 005740b0
	// $Barnyard: FUNCTION 0058c670
	// $Barnyard: FUNCTION 0062fae0
	// $Barnyard: FUNCTION 005c5c20
	T2DerivedDynamicObjectPool( T2Allocator* a_pAllocator, TINT a_iMaxNumber )
	{
		TVALIDPTR( a_pAllocator );
		m_iMaxNumber = a_iMaxNumber;
		m_pAllocator = a_pAllocator;

		TUINT32 uiClassSize      = const_cast<TClass*>( TClassType::GetClassStatic() )->GetMaxSizeOfDerivedClasses();
		TUINT32 uiClassAlignment = const_cast<TClass*>( TClassType::GetClassStatic() )->GetMaxAlignmentOfDerivedClasses();
		m_uiObjectSize           = ( uiClassSize - 1 ) + uiClassAlignment & ~( uiClassAlignment - 1U );

		TASSERT( m_uiObjectSize >= sizeof( TClassType ) && uiClassAlignment > 0 );

		if ( m_iMaxNumber < 1 )
		{
			m_pData = TNULL;
		}
		else
		{
			m_pData = m_pAllocator->Malloc( m_iMaxNumber * m_uiObjectSize, uiClassAlignment );
		}

		TVALIDPTR( m_pData );
		T2GenericObjectPool::Initialise(
		    TREINTERPRETCAST( T2GenericObjectPool::UnusedObject*, m_pData ),
		    m_iMaxNumber,
		    m_uiObjectSize
		);
	}

	// NOTE: retail has a second, derived pool vtable whose destructor only calls this one,
	// both deleting destructors are stacked here
	// $Barnyard: FUNCTION 00572150
	// $Barnyard: FUNCTION 00572120
	// $Barnyard: FUNCTION 00572330
	// $Barnyard: FUNCTION 005738c0
	// $Barnyard: FUNCTION 00573890
	// $Barnyard: FUNCTION 00573a50
	// $Barnyard: FUNCTION 00574090
	// $Barnyard: FUNCTION 00574060
	// $Barnyard: FUNCTION 005741a0
	// $Barnyard: FUNCTION 0058c650
	// $Barnyard: FUNCTION 0058c5d0
	// $Barnyard: FUNCTION 0058c9c0
	// $Barnyard: FUNCTION 0062fa30
	// $Barnyard: FUNCTION 0062fd50
	// $Barnyard: FUNCTION 005c5cb0
	// $Barnyard: FUNCTION 0062fa00
	virtual ~T2DerivedDynamicObjectPool()
	{
		m_pAllocator->Free( m_pData );
	}

	template <class... Args>
	TClassType* NewObject( Args&&... args )
	{
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
		TClassType* pValue = new ( Malloc( m_uiObjectSize ) ) TClassType( std::forward<Args>( args )... );
		return pValue;
	}

	template <class T, class... Args>
	T* NewDerivedObject( Args&&... args )
	{
		TASSERT( TTRUE == T::GetClassStatic()->IsA( TClassType::GetClassStatic() ) );
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
		T* pValue = new ( Malloc( m_uiObjectSize ) ) T( std::forward<Args>( args )... );
		return pValue;
	}

	TClassType* AllocateObject()
	{
		TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
		return TSTATICCAST( TClassType, Malloc( m_uiObjectSize ) );
	}

	void FreeObject( TClassType* a_pObject )
	{
		TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
		Free( a_pObject );
	}

	void DeleteObject( TClassType* a_pObject )
	{
		TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
		a_pObject->~TClassType();
		Free( a_pObject );
	}

	// NOTE: retail folds the T2DynamicObjectPool and T2DerivedDynamicObjectPool instantiations
	// $Barnyard: FUNCTION 0062f9e0
	virtual TSIZE GetUsedSize() const OVERRIDE
	{
		return ( m_iMaxNumber - GetNumFreeObjects() ) * m_uiObjectSize;
	}

	virtual TSIZE GetCapacity() const OVERRIDE
	{
		return m_iMaxNumber * m_uiObjectSize;
	}

	// $Barnyard: FUNCTION 00574030
	virtual TBOOL IsAddressInPool( const void* a_pAddress ) const
	{
		return TREINTERPRETCAST( TUINTPTR, m_pData ) <= TREINTERPRETCAST( TUINTPTR, a_pAddress ) &&
		    TREINTERPRETCAST( TUINTPTR, a_pAddress ) < TREINTERPRETCAST( TUINTPTR, m_pData ) + ( m_iMaxNumber * m_uiObjectSize );
	}

	TBOOL CanAllocate() const { return T2GenericObjectPool::CanAllocate( m_uiObjectSize ); }

private:
	T2Allocator* m_pAllocator;
	TINT         m_iMaxNumber;
	TUINT        m_uiObjectSize;
	void*        m_pData;
};

TOSHI_NAMESPACE_END
