#pragma once
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Purpose: one named property of an entity's data (AEntityParams.cpp), as stored in the level's entity TRB
// ("EntitiesMain").
//-----------------------------------------------------------------------------
struct AEntityParam
{
	enum TYPE : TINT
	{
		TYPE_INT32       = 0,
		TYPE_FLOAT       = 1,
		TYPE_VECTOR4     = 4,
		TYPE_COLLISION   = 8,  // a collision object (Collision*)
		TYPE_SOUNDACTION = 9,  // ASoundAction's data (ASoundAction::Data)
		TYPE_RESPAWN     = 11, // a respawn point: position and rotation (two TVector4, APropTriggerRespawn)
	};

	// A collision object shape (TYPE_COLLISION)
	struct CollisionShape
	{
		TINT              m_eType;         // 0x00 2: sphere (radius), 3: capsule, 4: cylinder (radius, height), 5: box (half extents)
		Toshi::TMatrix44* m_pMatrix;       // 0x04 the shape's offset
		TFLOAT            m_afValues[ 3 ]; // 0x08
	};

	// A collision object (TYPE_COLLISION, the value points to a pointer to it)
	struct Collision
	{
		TUINT32          m_uiUnk00;        // 0x00 -> TRBData 0x14
		TUINT32          m_uiUnk04;        // 0x04 -> TRBData 0x10
		TUINT32          m_aUnk08[ 2 ];    // 0x08
		TUINT32          m_uiGroupFlags;   // 0x10 collision groups (0x81, 2, 4, 8, 0x10, 0x20, 0x40, 0x100)
		TUINT32          m_uiPhysicsFlags; // 0x14 -> TRBData::m_uiPhysicsFlags
		TINT             m_iNumShapes;     // 0x18
		CollisionShape** m_ppShapes;       // 0x1C
	};

	// An array value (m_pArray)
	struct Array
	{
		TINT  m_iDataType; // 0x00
		TINT  m_iCount;    // 0x04
		void* m_pData;     // 0x08
	};

	// The elements of an array property, returns their count
	// $deBlob: FUNCTION 007c4db0
	TINT GetArray( TUINT32*& a_rData ) const
	{
		TASSERT( TYPE_INT32 == m_pArray->m_iDataType );
		a_rData = TSTATICCAST( TUINT32, m_pArray->m_pData );
		return m_pArray->m_iCount;
	}

	// $deBlob: FUNCTION 007c4e10
	TINT GetArray( TINT*& a_rData ) const
	{
		TASSERT( TYPE_INT32 == m_pArray->m_iDataType );
		a_rData = TSTATICCAST( TINT, m_pArray->m_pData );
		return m_pArray->m_iCount;
	}

	// $deBlob: FUNCTION 007c4e70
	TINT GetArray( TFLOAT*& a_rData ) const
	{
		TASSERT( TYPE_FLOAT == m_pArray->m_iDataType );
		a_rData = TSTATICCAST( TFLOAT, m_pArray->m_pData );
		return m_pArray->m_iCount;
	}

	// $deBlob: FUNCTION 007c4f30
	TINT GetDataType() const { return m_iDataType; }
	// $deBlob: FUNCTION 007c4ed0
	TINT GetArray( void*& a_rData ) const
	{
		TASSERT( TYPE_VECTOR4 == m_pArray->m_iDataType );
		a_rData = m_pArray->m_pData;
		return m_pArray->m_iCount;
	}

	const TCHAR* m_szName;    // 0x00
	TINT         m_iDataType; // 0x04 TYPE
	union
	{
		TINT             m_iInt32;
		TFLOAT           m_fFloat;
		const TCHAR*     m_szString;
		Toshi::TVector4* m_pVector4;
		Array*           m_pArray;
	}; // 0x08
};

//-----------------------------------------------------------------------------
// Purpose: the property list of an entity's data (AEntityParams.cpp), looked up by name (case-insensitive).
//-----------------------------------------------------------------------------
class AEntityParams
{
public:
	// $deBlob: FUNCTION 007c43a0
	const AEntityParam* FindParam( const TCHAR* a_szName ) const;

	// $deBlob: FUNCTION 007c43f0
	TBOOL GetValue( TINT& a_rValue, const TCHAR* a_szName ) const;
	// $deBlob: FUNCTION 007c4450
	TBOOL GetValue( TFLOAT& a_rValue, const TCHAR* a_szName ) const;
	TBOOL GetValue( TBOOL& a_rValue, const TCHAR* a_szName ) const;
	// $deBlob: FUNCTION 007c4550
	TBOOL GetValue( const TCHAR*& a_rValue, const TCHAR* a_szName ) const;
	TBOOL GetValue( Toshi::TVector4& a_rValue, const TCHAR* a_szName ) const;
	// $deBlob: FUNCTION 007c4650
	TBOOL GetValue( TUINT16& a_rHigh, TUINT16& a_rLow, const TCHAR* a_szName ) const;
	TBOOL GetValue( TUINT8& a_rValue, const TCHAR* a_szName ) const;
	TBOOL GetValue( TINT8& a_rValue, const TCHAR* a_szName ) const;
	TBOOL GetValue( TINT16& a_rValue, const TCHAR* a_szName ) const;

	// Returns the element count of an array property (-1 if there's none)
	// $deBlob: FUNCTION 007c48f0
	TINT GetArray( void*& a_rData, const TCHAR* a_szName ) const;

	// The same for an array of TINT (asserts TYPE_INT32)
	TINT GetArray( TINT*& a_rData, const TCHAR* a_szName ) const;
	TINT GetArray( TUINT32*& a_rData, const TCHAR* a_szName ) const;

	// The same for an array of TFLOAT (asserts TYPE_FLOAT)
	TINT GetArray( TFLOAT*& a_rData, const TCHAR* a_szName ) const;

	// Returns the value of the first property of type TYPE_COLLISION (by type, not by name)
	TBOOL GetCollision( AEntityParam::Collision**& a_rpCollision ) const;

	// Returns the value of the first property of type TYPE_SOUNDACTION (by type, not by name)
	TBOOL GetSoundAction( void*& a_rData ) const;

	// Returns the value of the first property of type TYPE_RESPAWN (by type, not by name)
	TBOOL GetRespawnPoint( void*& a_rData ) const;

	// An ID list property (types 5-7): the trigger instances, nav sets and camera shakes (ATriggerUtil)
	struct IDList
	{
		TINT* m_pIDs;   // 0x00
		TINT  m_iCount; // 0x04
	};

	// Return the value of the first property of type 5 (instances), 6 (nav sets) or 7 (camera shakes)
	TBOOL GetInstances( IDList*& a_rList ) const;
	TBOOL GetNavSets( IDList*& a_rList ) const;
	TBOOL GetCameraShakes( IDList*& a_rList ) const;

	// Typed forwarders
	// $deBlob: FUNCTION 007c4890
	TBOOL GetChar( TCHAR& a_rValue, const TCHAR* a_szName ) const { return GetValue( TREINTERPRETCAST( TINT8&, a_rValue ), a_szName ); }
	// $deBlob: FUNCTION 007c48a0
	TBOOL GetUInt( TUINT32& a_rValue, const TCHAR* a_szName ) const { return GetValue( TREINTERPRETCAST( TINT&, a_rValue ), a_szName ); }
	// $deBlob: FUNCTION 007c48b0
	TBOOL GetFloat( TFLOAT& a_rValue, const TCHAR* a_szName ) const { return GetValue( a_rValue, a_szName ); }
	// $deBlob: FUNCTION 007c48c0
	TBOOL GetBool( TBOOL& a_rValue, const TCHAR* a_szName ) const { return GetValue( a_rValue, a_szName ); }
	// $deBlob: FUNCTION 007c48d0
	TBOOL GetString( const TCHAR*& a_rValue, const TCHAR* a_szName ) const { return GetValue( a_rValue, a_szName ); }
	// $deBlob: FUNCTION 007c48e0
	TBOOL GetVector( Toshi::TVector4& a_rValue, const TCHAR* a_szName ) const { return GetValue( a_rValue, a_szName ); }

	TINT                GetNumParams() const { return m_iNumParams; }
	const AEntityParam* GetParams() const { return m_pParams; }

private:
	TINT FindIndex( const TCHAR* a_szName ) const;

protected:
	const TCHAR*  m_szTypeName; // 0x00 the entity type (AEntityFactory)
	TINT          m_iNumParams; // 0x04
	AEntityParam* m_pParams;    // 0x08
};
