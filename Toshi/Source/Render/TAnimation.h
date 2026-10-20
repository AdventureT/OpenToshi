#pragma once
#include "Math/TVector3.h"
#include "Math/TQuaternion.h"
#include "Toshi/TQList.h"
#include "File/TTRB.h"

#undef LoadLibrary

TOSHI_NAMESPACE_START

TCONSTEXPR_VAR TINT TANIMATION_MAXBONES = 128;

using TAnimVector     = TVector3;
using TAnimQuaternion = TQuaternion;
using TAnimScale      = TFLOAT;
using TAnimationBone  = unsigned short;

class TSkeletonInstance;

// The heap of TKeyframeLibrary and TSkeletonInstance (TAnimation.cpp), set by the game
void         SetTAnimMemoryHeap( TMemoryHeap* a_pHeap );
TMemoryHeap* GetTAnimMemoryHeap();

class TAnimation : public TQList<TAnimation>::TNode
{
public:
	using Flags = TUINT16;
	enum Flags_ : Flags
	{
		Flags_None                = 0,
		Flags_Active              = BITFLAG( 0 ),
		Flags_Managed             = BITFLAG( 1 ),
		Flags_Unknown2            = BITFLAG( 2 ),
		Flags_UpdateStateOnRemove = BITFLAG( 3 ),
	};

	using State = TUINT32;
	enum State_ : State
	{
		STATE_BLENDING_IN,
		STATE_PLAYING,
		STATE_BLENDING_OUT,
	};

public:
	TAnimation();
	~TAnimation();

	TBOOL UpdateTime( TFLOAT a_fDeltaTime );
	void  RemoveAnimation( TFLOAT a_fVal );

	void MoveAnimationToFront();
	void MoveAnimationToBack();

	// $deBlob: FUNCTION 005747a0
	void   SetState( State a_eState ) { m_eState = a_eState; }
	// $deBlob: FUNCTION 005747b0
	TFLOAT SetSpeed( TFLOAT a_fSpeed ) { return TExchange( m_fSpeed, a_fSpeed ); }
	TFLOAT SetDestWeight( TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed );
	void   SetBlendInSpeed( TFLOAT a_fBlendInSpeed ) { m_fBlendInSpeed = a_fBlendInSpeed; }
	void   SetBlendOutSpeed( TFLOAT a_fBlendOutSpeed ) { m_fBlendOutSpeed = a_fBlendOutSpeed; }

	// Sets the weight and switches to STATE_PLAYING, returns the previous weight
	TFLOAT SetWeight( TFLOAT a_fWeight );

	void ChangeToManaged( TFLOAT a_fBlendOutSpeed );

	// Returns TTRUE if the managed animation finishes blending out within a_fDeltaTime
	TBOOL WillFinishThisFrame( TFLOAT a_fDeltaTime ) const;

	// The highest TSkeletonSequence::m_iBoundingSphereIndex of the animations in a_rList (-1 if it's empty)
	static TINT __stdcall FindMaxBoundingSphereIndex( TQList<TAnimation>& a_rList );

	// Sets the time of the animation (wrapped or clamped to the sequence), returns the previous total time
	TFLOAT SetTime( TFLOAT a_fTime );

	Flags GetFlags() const { return m_eFlags; }
	State GetState() const { return m_eState; }
	TBOOL IsActive() const { return m_eFlags & Flags_Active; }
	TBOOL IsManaged() const { return m_eFlags & Flags_Managed; }
	TBOOL IsUpdateStateOnRemove() const { return m_eFlags & Flags_UpdateStateOnRemove; }

	// Sets or clears Flags_UpdateStateOnRemove, returns the previous state
	// $deBlob: FUNCTION 0068e060
	// $Barnyard: FUNCTION 006ca2a0
	TBOOL SetUpdateStateOnRemove( TBOOL a_bUpdateState )
	{
		TBOOL bOld = HASANYFLAG( m_eFlags, Flags_UpdateStateOnRemove );
		m_eFlags   = ( m_eFlags & ~Flags_UpdateStateOnRemove ) | ( ( a_bUpdateState == TTRUE ) ? Flags_UpdateStateOnRemove : 0 );
		return bOld;
	}

	TINT16                   GetSequence() const { return m_iSeqID; }
	class TSkeletonSequence* GetSequencePtr() const;
	TSkeletonInstance*       GetSkeletonInstance() const { return m_pSkeletonInstance; }
	// $deBlob: FUNCTION 0056f310
	TFLOAT                   GetSpeed() const { return m_fSpeed; }
	TFLOAT                   GetSeqTime() const { return m_fSeqTime; }
	TFLOAT                   GetTotalTime() const { return m_fTotalTime; }
	TFLOAT                   GetWeight() const { return m_fWeight; }
	TFLOAT                   GetDestWeight() const { return m_fDestWeight; }
	TFLOAT                   GetBlendInSpeed() const { return m_fBlendInSpeed; }
	TFLOAT                   GetBlendOutSpeed() const { return m_fBlendOutSpeed; }

	TAnimationBone* GetBones() { return TREINTERPRETCAST( TAnimationBone*, this + 1 ); }
	TAnimationBone* GetBone( TINT a_iIndex ) { return &TREINTERPRETCAST( TAnimationBone*, this + 1 )[ a_iIndex ]; }

private:
	TSkeletonInstance* m_pSkeletonInstance; // 0x08
	TINT16             m_iSeqID;            // 0x0C
	Flags              m_eFlags;            // 0x0E
	TINT16             m_iUnk3;             // 0x10
	State              m_eState;            // 0x14
	TFLOAT             m_fSpeed;            // 0x18
	TFLOAT             m_fWeight;           // 0x1C
	TFLOAT             m_fDestWeight;       // 0x20
	TFLOAT             m_fTotalTime;        // 0x24
	TFLOAT             m_fSeqTime;          // 0x28
	TFLOAT             m_fBlendInSpeed;     // 0x2C
	TFLOAT             m_fBlendOutSpeed;    // 0x30

	friend class TSkeletonInstance;
}; // 0x34, followed by a TAnimationBone (the current key) per auto bone

TSTATICASSERT( sizeof( TAnimation ) == 0x34 );

//-----------------------------------------------------------------------------
// Purpose: a keyframe library ("tkey" TAssetInit entries, Source\Render\TAnimation.cpp). de Blob's keeps a
// pointer to the TRB name and is allocated from TSkeleton's heap.
//-----------------------------------------------------------------------------
class TKeyframeLibrary : public TQList<TKeyframeLibrary>::TNode
{
public:
	friend class TKeyframeLibraryInstance;
	friend class TKeyframeLibraryManager;

	struct TRBHeader
	{
		TCHAR*           m_szName;
		TVector3         m_SomeVector;
		TINT             m_iNumTranslations;
		TINT             m_iNumQuaternions;
		TINT             m_iNumScales;
		TINT             m_iTranslationSize;
		TINT             m_iQuaternionSize;
		TINT             m_iScaleSize;
		TAnimVector*     m_pTranslations;
		TAnimQuaternion* m_pQuaternions;
		TAnimScale*      m_pScales;
	};

public:
	static TKeyframeLibrary* __stdcall Create( const TCHAR* a_szName, TINT a_iNumTranslations, TINT a_iNumQuaternions, TINT a_iNumScales, const TVector3& a_rSomeVector );
	static TKeyframeLibrary* __stdcall CreateFromData( TRBHeader* a_pTRBHeader );
	static TKeyframeLibrary* __stdcall CreateFromTRB( TTRB* a_pTRB, const TCHAR* a_szSymbolName );

	void Delete();
	void SetTRB( TTRB* a_pTRB ) { m_pTRB = a_pTRB; }

	const TCHAR* GetName() const { return m_szName; }

	TINT GetNumTranslations() const { return m_iNumTranslations; }
	TINT GetNumQuaternions() const { return m_iNumQuaternions; }
	TINT GetNumScales() const { return m_iNumScales; }

private:
	void IncreaseRefCount()
	{
		m_iReferenceCount++;
	}

	void DecreaseRefCount()
	{
		m_iReferenceCount--;
	}

private:
	const TCHAR*     m_szName;           // 0x08
	TVector3         m_SomeVector;       // 0x0C
	TINT             m_iReferenceCount;  // 0x18
	TINT             m_iNumTranslations; // 0x1C
	TINT             m_iNumQuaternions;  // 0x20
	TINT             m_iNumScales;       // 0x24
	TINT             m_iTranslationSize; // 0x28
	TINT             m_iQuaternionSize;  // 0x2C
	TINT             m_iScaleSize;       // 0x30
	TAnimVector*     m_pTranslations;    // 0x34
	TAnimQuaternion* m_pQuaternions;     // 0x38
	TAnimScale*      m_pScales;          // 0x3C
	TTRB*            m_pTRB;             // 0x40
}; // 0x44

class TKeyframeLibraryInstance
{
public:
	TKeyframeLibraryInstance();
	~TKeyframeLibraryInstance();

	TKeyframeLibrary* Create( TKeyframeLibrary* a_pLibrary );
	TKeyframeLibrary* CreateEx( TKeyframeLibrary* a_pLibrary, TINT a_iTKeyCount, TINT a_iQKeyCount, TINT a_iSKeyCount, TINT a_iTBaseIndex, TINT a_iQBaseIndex, TINT a_iSBaseIndex );

	void Destroy();

	// $deBlob: FUNCTION 0068fcd0
	TAnimVector* GetT( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iTKeyCount );
		return &m_pTranslations[ a_iIndex ];
	}

	// $deBlob: FUNCTION 0068fc40
	TAnimQuaternion* GetQ( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iQKeyCount );
		return &m_pQuaternions[ a_iIndex ];
	}

	TAnimScale GetS( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iQKeyCount );
		return m_pScales[ a_iIndex ];
	}

	void SetLibrary( TKeyframeLibrary* a_pLibrary ) { m_pLibrary = a_pLibrary; }
	void SetTCount( TINT a_iTCount ) { m_iTKeyCount = a_iTCount; }
	void SetQCount( TINT a_iQCount ) { m_iQKeyCount = a_iQCount; }
	void SetSCount( TINT a_iSCount ) { m_iSKeyCount = a_iSCount; }

	TKeyframeLibrary* GetLibrary() const { return m_pLibrary; }
	TINT              GetTCount() const { return m_iTKeyCount; }
	TINT              GetQCount() const { return m_iQKeyCount; }
	TINT              GetSCount() const { return m_iSKeyCount; }

private:
	TKeyframeLibrary* m_pLibrary;
	TINT              m_iTBaseIndex;
	TINT              m_iQBaseIndex;
	TINT              m_iSBaseIndex;
	TINT              m_iTKeyCount;
	TINT              m_iQKeyCount;
	TINT              m_iSKeyCount;
	TAnimVector*      m_pTranslations;
	TAnimQuaternion*  m_pQuaternions;
	TAnimScale*       m_pScales;
};

class TKeyframeLibraryManager
{
public:
	TKeyframeLibraryManager();
	~TKeyframeLibraryManager();

	TKeyframeLibrary* GetLibrary( const TCHAR* a_szName );

	TKeyframeLibrary* LoadLibrary( const TCHAR* a_szFilePath );
	TKeyframeLibrary* LoadLibrary( TTRB* a_pTRB, const TCHAR* a_szSymbolName );
	TKeyframeLibrary* LoadLibrary( TKeyframeLibrary::TRBHeader* a_pTRBHeader );

	void UnloadLibrary( TKeyframeLibrary* a_pLibrary );

private:
	TQList<TKeyframeLibrary> m_List;              // 0x00
	TINT                     m_iNumLibraries = 0; // 0x08
}; // 0x0C

TOSHI_NAMESPACE_END
