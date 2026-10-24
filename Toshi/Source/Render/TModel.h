/**
 * @file TModel.h
 * 3D model system for the Toshi engine
 *
 * de Blob's TModel (Source\Render\TModel.cpp, the platform half in TModel_DX11.cpp): a T2Resource made from a model
 * TRB, with up to 5 LODs of meshes, an optional skeleton and collision data. Models are linked into TModelManager
 * and referenced through T2ModelPtr. Names from Globs of Doom (NT08 TModel.o, the Wii version).
 */

#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "TMesh.h"
#include "TTransformObject.h"
#include "TModelCollision.h"

#include "Math/TSphere.h"
#include "File/TTRB.h"
#include "Toshi/T2ResourceHelper.h"

class AModelLoader;
class AModel2;
class ACollisionInstanceMover;

TOSHI_NAMESPACE_START

class TModelCollision;
class TModelInstance;

//-----------------------------------------------------------------------------
// Purpose: one level of detail: a bounding sphere and the meshes
//-----------------------------------------------------------------------------
struct TModelLOD
{
	// $deBlob: FUNCTION 006b0850
	TModelLOD()
	    : m_RenderVolume( 0.0f, 0.0f, 0.0f, 0.0f ), m_iMeshCount( 0 ), m_pMeshes( TNULL )
	{}

	TSphere GetRenderVolume() const { return m_RenderVolume; }
	TINT    GetMeshCount() const { return m_iMeshCount; }
	TMesh** GetMeshes() const { return m_pMeshes; }

	TMesh* GetMesh( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iMeshCount );
		return m_pMeshes[ a_iIndex ];
	}

	TSphere m_RenderVolume; // 0x00
	TINT    m_iMeshCount;   // 0x10
	TMesh** m_pMeshes;      // 0x14
}; // 0x18

class TModel : public T2Resource
{
public:
	using Flags = TUINT32;
	enum Flags_ : Flags
	{
		Flags_None      = 0,
		Flags_Created   = BITFLAG( 0 ),
		Flags_TrbLoaded = BITFLAG( 1 ),
	};

	using t_TRBLoadCallback = TBOOL( __stdcall* )( TTRB* a_pTRB, TModel* a_pModel );

	static TCONSTEXPR_VAR TINT MAX_NUM_LODS = 5;

	friend class TModelManager;
	friend class TModelInstance;
	friend class TAssetInit;
	friend class ::AModelLoader;

public:
	TModel();
	~TModel();

	// TModel_DX11.cpp
	TBOOL Create( const TCHAR* a_szName, TBOOL a_bLoadImmediately );
	void  Delete();
	void  Unload();

	// $deBlob: FUNCTION 006af440
	void SetTRB( TTRB* a_pTRB, TBOOL a_bFreeOnUnload )
	{
		m_pTRB          = a_pTRB;
		m_bFreeOnUnload = a_bFreeOnUnload;
	}

	TBOOL LoadTRB();
	TBOOL LoadTRB( TTRB* a_pTRB, TBOOL a_bFreeOnUnload );
	TBOOL LoadTRBFile( const TCHAR* a_szFileName );
	TBOOL LoadTRBFile( TFile* a_pFile );
	void  UnloadTRB( TBOOL a_bFreeTRB );

	TModelInstance* CreateInstance();

	TBOOL IsCreated() const { return m_eFlags & Flags_Created; }
	TBOOL IsTRBLoaded() const { return m_eFlags & Flags_TrbLoaded; }

	// $deBlob: FUNCTION 006b0880
	TINT GetLODCount() const { return m_iLODCount; }
	// $deBlob: FUNCTION 006b0970
	void SetLODCount( TINT a_iLODCount ) { m_iLODCount = a_iLODCount; }

	// Out-of-line instance:
	// $deBlob: FUNCTION 0054efb0
	TModelLOD& GetLOD( TINT a_iLODLevel )
	{
		TASSERT( a_iLODLevel < m_iLODCount );
		return m_LODs[ a_iLODLevel ];
	}

	// $deBlob: FUNCTION 006b08c0
	TFLOAT GetLODDistance( TINT a_iLODLevel ) const
	{
		TASSERT( a_iLODLevel > 0 && a_iLODLevel < m_iLODCount );
		return m_aLODDistances[ a_iLODLevel - 1 ];
	}

	// $deBlob: FUNCTION 005a6080
	const TCHAR*     GetName() const { return m_szName; }
	TTRB*            GetTRB() const { return m_pTRB; }
	TSkeleton*       GetSkeleton() const { return m_pSkeleton; }
	TModelCollision* GetCollision() const { return m_pCollision; }

	TINT GetCollisionDataCount() const { return m_pCollision->m_iNumCollisionModels; }

	// TModel.h:135
	// $deBlob: FUNCTION 005a5ec0
	TModelCollisionMesh& GetCollisionData( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_pCollision->m_iNumCollisionModels );
		return m_pCollision->m_pCollisionModels[ a_iIndex ];
	}
	TINT             GetNumInstances() const { return m_iNumInstances; }
	void*            GetAssetData() const { return m_pAssetData; }
	void             SetAssetData( void* a_pAssetData ) { m_pAssetData = a_pAssetData; }

	template <class T>
	T* CastSymbol( const TCHAR* a_szSymbolName ) { return TSTATICCAST( T, m_pTRB->GetSymbolAddress( TranslateSymbolName( a_szSymbolName ) ) ); }

protected:
	void CreateResource( const TCHAR* a_szName );
	void CreateSkeleton( TTMDBase::SkeletonHeader* a_pSkeletonHeader, TSkeleton* a_pSkeleton, TBOOL a_bLoadAnimations );
	void CreateCollision( TModelCollision* a_pCollision );

public:
	static void* __stdcall ResourceCallback( void* a_pData, TTRB* a_pTRB, TBOOL a_bCreated );
	static const TCHAR* __stdcall TranslateSymbolName( const TCHAR* a_szSymbolName );

	// $deBlob: FUNCTION 005f9ae0
	static void SetTRBLoadCallback( t_TRBLoadCallback a_fnCallback ) { sm_pTRBLoadCallback = a_fnCallback; }

	static TMemoryHeap* GetMemHeap() { return s_pMemHeap; }
	static void         SetMemHeap( TMemoryHeap* a_pHeap ) { s_pMemHeap = a_pHeap; }

protected:
	static t_TRBLoadCallback sm_pTRBLoadCallback;
	static const TCHAR*      sm_SymbolNamePrefix; // (never set)
	static TMemoryHeap*      s_pMemHeap;          // TModelInstance heap, set by the game

protected:
	Flags            m_eFlags;                        // 0x04
	TINT             m_iNumInstances;                 // 0x08
	TINT             m_iLODCount;                     // 0x0C
	TFLOAT           m_fUnknown;                      // 0x10 -1.0f, set from the TRB header
	TSkeleton*       m_pSkeleton;                     // 0x14
	TModelLOD        m_LODs[ MAX_NUM_LODS ];          // 0x18
	TFLOAT           m_aLODDistances[ MAX_NUM_LODS - 1 ]; // 0x90
	TModelCollision* m_pCollision;                    // 0xA0
	TTRB*            m_pTRB;                          // 0xA4
	TBOOL            m_bFreeOnUnload;                 // 0xA8
	const TCHAR*     m_szName;                        // 0xAC
	T2ModelPtr       m_pNextModel;                    // 0xB0 TModelManager list
	T2ModelPtr       m_pPrevModel;                    // 0xB4
	void*            m_pAssetData;                    // 0xB8 the TAssetInit "tmod"/"twld" data it was made from
}; // 0xBC

//-----------------------------------------------------------------------------
// Purpose: a renderable instance of a TModel
//-----------------------------------------------------------------------------
class TModelInstance
{
public:
	using t_PreRenderCB    = void( __stdcall* )( TModelInstance* a_pInstance );
	using t_CustomRenderCB = void( __stdcall* )( TModelInstance* a_pInstance, void* a_pUserData );

	using Flags = TUINT32;
	enum Flags_ : Flags
	{
		Flags_None               = 0,
		Flags_Visible            = BITFLAG( 0 ),
		Flags_DistanceFade       = BITFLAG( 1 ),
		Flags_ForceFade          = BITFLAG( 2 ),
		Flags_Created            = BITFLAG( 3 ),
		Flags_BeforeZBufferClear = BITFLAG( 4 ), // TScene renders it before the depth buffer is cleared
	};

	friend class TModel;
	friend class ::AModel2; // sets the distance fade of the instances it creates
	friend class ::ACollisionInstanceMover; // shows/hides the instance it moves

public:
	TModelInstance( T2ModelPtr a_pModel );
	~TModelInstance();

	void  Update( TFLOAT a_fDeltaTime );
	void  Render();
	TBOOL IsInViewFrustum();
	void  CalculateWorldSpacePosition( TVector4& a_rOutPosition );

	void SetUVHashOffset( void* a_pUVHashOffset );

	void SetTextureStages( TINT8 a_iStage0, TINT8 a_iStage1, TINT8 a_iStage2, TINT8 a_iStage3 );
	void SetTextureStagesA( TINT8* a_pStages );
	void GetTextureStages( TINT8* a_pOutStages );
	void SetTextureAlphas( TUINT8 a_uiAlpha0, TUINT8 a_uiAlpha1, TUINT8 a_uiAlpha2, TUINT8 a_uiAlpha3 );
	void SetTextureAlphasA( TUINT8* a_pAlphas );
	void GetTextureAlphas( TUINT8* a_pOutAlphas );

	void SetPreRenderCallback( t_PreRenderCB a_fnCallback ) { m_fnPreRenderCb = a_fnCallback; }

	void SetCustomRenderMethod( t_CustomRenderCB a_fnCallback, void* a_pUserData )
	{
		m_fnCustomRenderCb        = a_fnCallback;
		m_pCustomRenderCbUserData = a_pUserData;
	}

	t_CustomRenderCB GetCustomRenderCb() const { return m_fnCustomRenderCb; }
	void*            GetCustomRenderCbUserData() const { return m_pCustomRenderCbUserData; }
	TBOOL            IsVisible() const { return ( m_eFlags & Flags_Visible ) != 0; }

	// $deBlob: FUNCTION 006b0960
	TBOOL IsCreated() const { return ( m_eFlags & Flags_Created ) != 0; }
	// $deBlob: FUNCTION 006b08a0
	TBOOL IsDistanceFade() const { return ( m_eFlags & Flags_DistanceFade ) != 0; }
	Flags GetFlags() const { return m_eFlags; }

	// NOTE: retail returns a copy (callers release the temporary)
	// $deBlob: FUNCTION 0054f0c0
	T2ModelPtr         GetModel() const { return m_pModel; }
	TSkeletonInstance* GetSkeletonInstance() const { return m_pSkeletonInstance; }
	TINT               GetLOD() const { return m_iCurrentLOD; }

	// de Blob: returns the previous LOD
	// $deBlob: FUNCTION 00555030
	TINT SetLOD( TINT a_iLOD )
	{
		TINT iOldLOD = m_iCurrentLOD;
		TASSERT( a_iLOD < m_pModel->GetLODCount() );
		m_iCurrentLOD = a_iLOD;
		return iOldLOD;
	}
	TTransformObject&  GetTransform() { return m_TransformObject; }

	void SetDistanceFade( TBOOL a_bEnable )
	{
		if ( a_bEnable ) m_eFlags |= Flags_DistanceFade;
		else m_eFlags &= ~Flags_DistanceFade;
	}

	void SetForceFade( TBOOL a_bForce )
	{
		if ( a_bForce ) m_eFlags |= Flags_ForceFade;
		else m_eFlags &= ~Flags_ForceFade;
	}

	void SetFade( TFLOAT a_fFade ) { m_fFade = a_fFade; }
	TFLOAT GetFade() const { return m_fFade; }
	TBOOL  IsForceFade() const { return ( m_eFlags & Flags_ForceFade ) != 0; }

private:
	TBOOL TGFXFileInstanceObjectPreRender();
	void  TGFXFileInstanceObjectPostRender();

private:
	static TFLOAT s_fOldLightTexMag;
	static TUINT  s_iOldClipFlags;

private:
	T2ModelPtr         m_pModel;                  // 0x00
	TSkeletonInstance* m_pSkeletonInstance;       // 0x04
	t_PreRenderCB      m_fnPreRenderCb;           // 0x08
	t_CustomRenderCB   m_fnCustomRenderCb;        // 0x0C
	void*              m_pCustomRenderCbUserData; // 0x10
	TINT               m_iCurrentLOD;             // 0x14
	void*              m_pUVHashOffset;           // 0x18
	TTransformObject   m_TransformObject;         // 0x1C
	Flags              m_eFlags;                  // 0xC0
	TINT8  m_aTextureStages[ 4 ]; // 0xC4
	TUINT8 m_aTextureAlphas[ 4 ]; // 0xC8
	TFLOAT m_fFadeStart;          // 0xCC
	TFLOAT m_fFadeEnd;            // 0xD0
	TFLOAT m_fFadeRangeInv;       // 0xD4
	TFLOAT m_fFade;               // 0xD8
	TFLOAT m_fLightTexMag;        // 0xDC
}; // 0xE0

TOSHI_NAMESPACE_END
