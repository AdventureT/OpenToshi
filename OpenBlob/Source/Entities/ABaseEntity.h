#pragma once
#include <Toshi/TObject.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2Map.h>
#include <Toshi/TString8.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>
#include "Entities/AEntityIOPacket.h"

class ABaseEntity;
class ACollisionObject;
class AEntityParams;
struct AEntityTypeData;
struct AEntityHashRef;

//-----------------------------------------------------------------------------
// Purpose: links an entity to the terrain cell it's in, to the factory's always-tick/unlinked lists and to its stale
// list (AEntityFactory.h, ABaseEntity::m_pEntityCellPair).
//-----------------------------------------------------------------------------
struct ACellPair
    : public Toshi::T2DList<ACellPair>::Node
{
	using StaleNode = Toshi::T2DListNodeWrapper<ACellPair>;

	ACellPair( ABaseEntity* a_pEntity )
	    : m_pEntity( a_pEntity ), m_pCell( TNULL ), m_oStaleNode( this )
	{}

	// NOTE: the node destructors only assert that both nodes are unlinked
	// $deBlob: FUNCTION 0075f120
	// $deBlob: FUNCTION 0075f430
	~ACellPair() = default;

	ABaseEntity* m_pEntity;    // 0x08
	void*        m_pCell;      // 0x0C the terrain cell
	StaleNode    m_oStaleNode; // 0x10 (its value, at 0x18, is this pair)
};

//-----------------------------------------------------------------------------
// Purpose: the base of every entity: a TObject created by AEntityFactory from the TClass named by its entity type, with
// a matrix, a bounding box, a state, an ID (sm_EntityHash), flags and its cell pair.
//-----------------------------------------------------------------------------
class ABaseEntity
    : public Toshi::TObject
    , public Toshi::T2DList<ABaseEntity>::Node
{
public:
	TDECLARE_CLASS( ABaseEntity, Toshi::TObject );

	static TCONSTEXPR_VAR TINT ENTITY_HASH_SIZE = 1024;

	enum FLAG : TUINT32
	{
		FLAG_Created             = 0x1,
		FLAG_Hashed              = 0x2, // in AEntityHash
		FLAG_Visible             = 0x4,
		FLAG_Player              = 0x10,  // AAISteer::AdjustSuggestions: an ABlobEntity (its velocity comes from the body)
		FLAG_Ticking             = 0x100, // SetTicking
		FLAG_LOD0                = 0x200,
		FLAG_LowLOD              = 0x200, // = FLAG_LOD0
		FLAG_MediumLOD           = 0x400,
		FLAG_HighLOD             = 0x800,
		FLAG_LODMask_Level       = 0xE00, // GetLOD's mask
		FLAG_LODMask             = 0x1E00,
		FLAG_InCell              = 0x2000,
		FLAG_DontTick            = 0x4000,
		FLAG_UpdateMatrix        = 0x8000,
		FLAG_Pickup              = 0x10000, // ticked by AEntityFactory::Tick( .., 1 ), in the pickups of AEntityHash
		FLAG_PhysicsEnabled      = 0x200000,
		FLAG_Dead                = 0x400000,
		FLAG_Pooled              = 0x800000,
		FLAG_Despawned           = 0x1000000, // skipped by AEntityHash's queries (stamped, never returned)
		FLAG_IgnoreCellForLOD    = 0x10000000,
		FLAG_UseLargeLODDistance = 0x20000000,
	};

	// Case-insensitive (TString8::CompareNoCase)
	struct NameComparator
	{
		TINT operator()( const Toshi::TString8& a_rcA, const Toshi::TString8& a_rcB ) const { return a_rcA.CompareNoCase( a_rcB.GetString(), -1 ); }
	};

	using NameMap = Toshi::T2Map<Toshi::TString8, TINT, NameComparator>;

public:
	ABaseEntity();
	// $deBlob: FUNCTION 0075f400
	virtual ~ABaseEntity() override; // +0x08

	// $deBlob: FUNCTION 0075e8b0
	virtual void Tick( TFLOAT a_fDeltaTime ) { m_fStateTime += a_fDeltaTime; } // +0x0C
	// $deBlob: FUNCTION 00751e50
	virtual void OnLODChanging( TINT a_iLOD ) {} // +0x10
	// $deBlob: FUNCTION 00752390
	virtual void SyncBodyTransform() {} // +0x14 called by AEntityLoader once the entity is placed
	// $deBlob: FUNCTION 0075e8d0
	virtual void OnOutOfWorld() {} // +0x18
	virtual void Create();         // +0x1C
	// $deBlob: FUNCTION 00751ee0
	virtual void PostCreate() {} // +0x20 called by AEntityLoader once every entity is created
	virtual void Destroy();      // +0x24
	// $deBlob: FUNCTION 00751e00
	virtual void OnDataLoad() {}                                  // +0x28
	virtual void ProcessParams( const AEntityParams& a_rParams ); // +0x2C
	// $deBlob: FUNCTION 00751ed0
	virtual void InitPooled( TINT a_eType ) {} // +0x30
	// $deBlob: FUNCTION 007516d0
	virtual TFLOAT GetAnimationSpeed( TINT a_iUnk ) const { return 1.0f; } // +0x34
	// $deBlob: FUNCTION 00752310
	virtual void SetVisible( TBOOL a_bVisible ) // +0x38
	{
		if ( a_bVisible )
			m_Flags |= FLAG_Visible;
		else
			m_Flags &= ~FLAG_Visible;
	}
	// $deBlob: FUNCTION 00751cf0
	virtual TBOOL IsDead() const
	{
		TBOOL bDead = ( m_Flags >> 22 ) & 1;
		return bDead;
	} // +0x3C
	// $deBlob: FUNCTION 00752250
	virtual void SetPosition( const Toshi::TVector4& a_rPosition ) // +0x40
	{
		m_oMatrix.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION ) = a_rPosition;
		m_Flags |= FLAG_UpdateMatrix;
	}
	// $deBlob: FUNCTION 0075f770
	virtual void SetFacing( const Toshi::TVector4& a_rFacing ) // +0x44
	{
		m_oMatrix.LookAtDirection( a_rFacing, Toshi::TVector4::VEC_NEGY );
		m_Flags |= FLAG_UpdateMatrix;
	}
	// $deBlob: FUNCTION 007521e0
	virtual void SetMatrix( const Toshi::TMatrix44& a_rMatrix ) // +0x48
	{
		m_oMatrix = a_rMatrix;
		m_Flags |= FLAG_UpdateMatrix;
	}
	// $deBlob: FUNCTION 007518f0
	virtual void* GetModelComponent() { return TNULL; } // +0x4C
	// $deBlob: FUNCTION 00751930
	virtual void* GetPoseController() { return TNULL; } // +0x50
	// $deBlob: FUNCTION 00751900
	virtual void* GetPathfinding() { return TNULL; } // +0x54
	// $deBlob: FUNCTION 00751a70
	virtual void* GetStateComponent() { return TNULL; } // +0x58
	// $deBlob: FUNCTION 0075f510
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) {} // +0x5C
	// $deBlob: FUNCTION 00751950
	virtual Toshi::TVector4 GetReactionForce( void* a_pVelocity, void* a_pEntity ) { return Toshi::TVector4::VEC_ZERO; } // +0x60
	// $deBlob: FUNCTION 00751970
	virtual Toshi::TVector4 GetReactionRotation( void* a_pEntity ) { return Toshi::TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); } // +0x64
	// $deBlob: FUNCTION 00751ec0
	virtual void OnTouched( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 ) {} // +0x68
	// $deBlob: FUNCTION 00751ef0
	virtual void PreRenderUpdate() {} // +0x6C
	// $deBlob: FUNCTION 0075f4f0
	virtual ACollisionObject* GetCollisionObject() { return TNULL; } // +0x70
	// $deBlob: FUNCTION 00751a80
	virtual TFLOAT GetStepUpHeight() const { return 0.3f; } // +0x74
	// $deBlob: FUNCTION 00751b80
	virtual TBOOL HandleFallenThroughWorld() { return TFALSE; } // +0x78
	// $deBlob: FUNCTION 0075eb60
	virtual void SetBBox( Toshi::TVector4 a_vBBox ) // +0x7C
	{
		m_vBBox                 = a_vBBox;
		m_fBoundingSphereRadius = a_vBBox.Magnitude();
	}
	// The bottom of the bounding box: the position moved down the up axis by the box's half height
	// $deBlob: FUNCTION 0075f490
	virtual Toshi::TVector4 GetBasePosition() const; // +0x80

	// The translation of m_oMatrix (0x40 of the entity)
	const Toshi::TVector4& GetTranslation() const { return m_oMatrix.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION ); }
	// $deBlob: FUNCTION 00751b90
	virtual TBOOL IgnoreProjectileCollision( void* a_pProjectile ) { return TFALSE; } // +0x84
	// $deBlob: FUNCTION 007514b0
	virtual void AddToPhysics() {} // +0x88
	// $deBlob: FUNCTION 00751f20
	virtual void RemoveFromPhysics() {} // +0x8C
	// $deBlob: FUNCTION 007520c0
	virtual void SetPhysicsEnabled( TBOOL a_bEnable ) // +0x90
	{
		if ( a_bEnable )
			m_Flags |= FLAG_PhysicsEnabled;
		else
			m_Flags &= ~FLAG_PhysicsEnabled;
	}
	// $deBlob: FUNCTION 00751c90
	virtual TBOOL IsInteractable() { return TFALSE; } // +0x94
	// $deBlob: FUNCTION 00751820
	virtual TINT GetInteractionPriority() { return 10; } // +0x98
	// $deBlob: FUNCTION 00751830
	virtual TFLOAT GetInteractionRadius() const { return 2.5f; } // +0x9C
	// $deBlob: FUNCTION 0075f710
	virtual TBOOL IsTargetable() { return TFALSE; } // +0xA0
	// $deBlob: FUNCTION 007514e0
	virtual TBOOL CanBeTargetedFromBelow() { return TFALSE; } // +0xA4
	// $deBlob: FUNCTION 00751d80
	virtual TINT GetTargettingPriority() { return 0; } // +0xA8
	// $deBlob: FUNCTION 00752370
	virtual Toshi::TVector4 GetTargetOffset() { return Toshi::TVector4::VEC_ZERO; } // +0xAC
	// $deBlob: FUNCTION 007516e0
	virtual TFLOAT GetAttackSlamHeight() { return 0.0f; } // +0xB0
	// $deBlob: FUNCTION 00751b70
	virtual TINT GetTargetColour() { return 0; } // +0xB4
	// $deBlob: FUNCTION 00751a40
	virtual TINT GetMaxLife() { return 0; } // +0xB8
	// $deBlob: FUNCTION 007514d0
	virtual TBOOL CanBlobStick() { return TFALSE; } // +0xBC
	// $deBlob: FUNCTION 00752240
	virtual void SetPoolOwner( TINT a_iOwner ) {} // +0xC0
	// $deBlob: FUNCTION 00751920
	virtual TINT GetPoolOwner() { return 0; } // +0xC4
	// Back to the first LOD
	// $deBlob: FUNCTION 00751f50
	virtual void Reset( ABaseEntity* a_pEntity ) { m_Flags = ( m_Flags & ~FLAG_LODMask ) | FLAG_LOD0; } // +0xC8
	// $deBlob: FUNCTION 007520b0
	virtual void SetActive( TBOOL a_bActive ) {} // +0xCC
	// $deBlob: FUNCTION 007514c0
	virtual void OnPoolRelease() {} // +0xD0
	// $deBlob: FUNCTION 00751e60
	virtual void OnLeaveState( TINT a_iState ) {} // +0xD4
	// $deBlob: FUNCTION 00751e10
	virtual void OnEnterState( TINT a_iState ) {} // +0xD8

	void SetState( TINT a_State );
	void SetTicking( TBOOL a_bTicking );
	void SetHash( TBOOL a_bHashed );

	TBOOL IsVisible() const { return ( m_Flags & FLAG_Visible ) != 0; }
	TBOOL IsDespawned() const { return ( m_Flags & FLAG_Despawned ) != 0; }
	void  SetName( const TCHAR* a_szName );

	TBOOL HasStateFinished();
	TBOOL HasStateTimedOut();
	TBOOL HasPoseFinished();

	// $deBlob: FUNCTION 005999a0
	void SetID( TINT a_iID )
	{
		TASSERT( !IsPooled() );
		TASSERT( a_iID >= 0 && a_iID < ENTITY_HASH_SIZE, "Invalid entity hash ID" );
		TASSERT( sm_EntityHash[ a_iID ] == 0, "Entity hash error, conflicting entity IDs" );

		if ( m_iID >= 0 )
		{
			TASSERT( m_iID >= 0 && m_iID < ENTITY_HASH_SIZE );
			TASSERT( sm_EntityHash[ m_iID ] == this );
			sm_EntityHash[ m_iID ] = TNULL;
		}

		m_iID                  = TINT16( a_iID );
		sm_EntityHash[ a_iID ] = this;
	}

	TBOOL IsPooled() const { return ( m_Flags & FLAG_Pooled ) != 0; }

	// Pooled entities (AEnemyManager) get IDs above the hash
	// $deBlob: FUNCTION 0078c770
	void SetPooledID()
	{
		TASSERT( IsPooled() );
		m_iID = TINT16( ms_iNextPooledID );
		ms_iNextPooledID++;

		if ( ms_iNextPooledID < 0 )
			ms_iNextPooledID = ENTITY_HASH_SIZE;
	}

	// $deBlob: FUNCTION 0078c050
	void ClearPooledID()
	{
		TASSERT( IsPooled() );
		m_iID = -1;
	}

	// The ID of an entity in the hash (not pooled)
	// $deBlob: FUNCTION 0076dd80
	TINT16 GetHashID() const
	{
		TASSERT( !IsPooled() );
		return m_iID;
	}

	// The ID of a pooled entity (the 16-bit m_iID at 0x0E)
	// $deBlob: FUNCTION 0076a9b0
	TINT16 GetPooledID() const
	{
		TASSERT( IsPooled() );
		return m_iID;
	}

	// FLAG_LowLOD (FLAG_LOD0), FLAG_MediumLOD or FLAG_HighLOD
	// $deBlob: FUNCTION 0057b4d0
	TUINT32 GetLOD() const
	{
		TASSERT( ( FLAG_LODMask_Level & m_Flags ) == FLAG_LowLOD || ( FLAG_LODMask_Level & m_Flags ) == FLAG_MediumLOD || ( FLAG_LODMask_Level & m_Flags ) == FLAG_HighLOD );
		return m_Flags & FLAG_LODMask_Level;
	}
	TBOOL IsFlagged( TUINT32 a_uiFlag ) const { return ( m_Flags & a_uiFlag ) != 0; }

	// Sets or clears a flag; the tick and collision flags have their own setters.
	// $deBlob: FUNCTION 00752100
	void SetFlag( TUINT32 a_Flag, TBOOL a_bSet )
	{
		TASSERT( FLAG_Ticking != a_Flag );
		TASSERT( FLAG_PhysicsEnabled != a_Flag );

		if ( a_bSet )
			m_Flags |= a_Flag;
		else
			m_Flags &= ~a_Flag;
	}

	TINT                    GetState() const { return m_iState; }
	TINT                    GetID() const { return m_iID; }
	TUINT32                 GetFlags() const { return m_Flags; }
	void                    SetFlags( TUINT32 a_uiFlags ) { m_Flags = a_uiFlags; }
	const Toshi::TMatrix44& GetMatrix() const { return m_oMatrix; }
	const Toshi::TVector4&  GetBBox() const { return m_vBBox; }
	// $deBlob: FUNCTION 00751850
	TINT   GetLife() const { return m_iLife; }
	TINT16 GetCollisionGroupAndType() const { return m_iCollisionGroupAndType; }
	// $deBlob: FUNCTION 00751940
	TFLOAT GetBoundingSphereRadius() const { return m_fBoundingSphereRadius; }
	// $deBlob: FUNCTION 00752280
	void             SetBoundingSphereRadius( TFLOAT a_fRadius ) { m_fBoundingSphereRadius = a_fRadius; }
	AEntityTypeData* GetTypeData() const { return m_pTypeData; }
	void             SetTypeData( AEntityTypeData* a_pTypeData ) { m_pTypeData = a_pTypeData; }
	ACellPair&       GetCellPair() { return *m_pEntityCellPair; }
	// $deBlob: FUNCTION 007741d0
	TUINT32 GetHashTimeStamp() const { return m_uiHashTimeStamp; }
	void    SetHashTimeStamp( TUINT32 a_uiTimeStamp ) { m_uiHashTimeStamp = a_uiTimeStamp; }
	// $deBlob: FUNCTION 007741c0
	AEntityHashRef* GetHashRef() const { return m_pHashRef; }
	// $deBlob: FUNCTION 00774300
	void SetHashRef( AEntityHashRef* a_pHashRef ) { m_pHashRef = a_pHashRef; }

	// Returns the ID named a_szName (SetName), or -1
	static TINT GetIDName( const TCHAR* a_szName );

	// $deBlob: FUNCTION 00578450
	static ABaseEntity* FindIDEntity( TINT a_iID )
	{
		TASSERT( a_iID >= 0 && a_iID < ENTITY_HASH_SIZE );
		return sm_EntityHash[ a_iID ];
	}

private:
	static ABaseEntity* sm_EntityHash[ ENTITY_HASH_SIZE ];
	static NameMap      sm_oNameMap;
	static TINT         ms_iNextPooledID;

protected:
	TINT16           m_iState;                 // 0x0C
	TINT16           m_iID;                    // 0x0E
	Toshi::TMatrix44 m_oMatrix;                // 0x10
	Toshi::TVector4  m_vBBox;                  // 0x50
	TINT16           m_iLife;                  // 0x60
	TINT16           m_iCollisionGroupAndType; // 0x62
	TUINT32          m_uiHashTimeStamp;        // 0x64 AEntityHash::NextTimeStamp
	TFLOAT           m_fBoundingSphereRadius;  // 0x68
	TFLOAT           m_fStateTime;             // 0x6C
	TUINT32          m_Flags;                  // 0x70 FLAG
	AEntityTypeData* m_pTypeData;              // 0x74
	AEntityHashRef*  m_pHashRef;               // 0x78
	ACellPair*       m_pEntityCellPair;        // 0x7C
};
