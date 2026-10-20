#pragma once
#include <Toshi/T2DList.h>
#include <Render/TModel.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

class AModel2;
class AToshiAnimInterface2;

//-----------------------------------------------------------------------------
// Purpose: an instance of an AModel2: owns the TModelInstance once the model is loaded, the animation interface, the
// sound breakpoint component and the collision instance.
// Created by AModel2::CreateInstance (also ABuildingInstance, ALandmarkInstance, ATreeInstance, AGrassInstance).
//-----------------------------------------------------------------------------
class AModelInstance2
    : public Toshi::T2DList<AModelInstance2>::Node
{
public:
	using FLAG = TUINT16;
	enum FLAG_ : FLAG
	{
		FLAG_Visible               = BITFLAG( 0 ),
		FLAG_LocalAmbient          = BITFLAG( 1 ),
		FLAG_LocalAmbientColour    = BITFLAG( 2 ),
		FLAG_RenderOnLoadingScreen = BITFLAG( 3 ), // rendered by the loading screen (AModel2::RenderLoadingScreenInstances)
		FLAG_NoAnimSounds          = BITFLAG( 10 ),
		FLAG_Renderable            = BITFLAG( 11 ),
	};

	// The default local ambient colour (dword_97B4C8)
	static TUINT32 ms_uiDefaultLocalAmbientColour;

	// The number of instances rendered this frame (reset by ARenderer::MainScene)
	static TINT ms_RenderCount;

	friend class AModel2;
	friend class APropHazard; // its render method (APropHazard::RenderModelCallback) is a copy of RenderGeneric

	static TCONSTEXPR_VAR TINT MAX_MESH_SETTINGS = 5;

public:
	AModelInstance2( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints, TBOOL a_bCreateAnimInterface );

	virtual void UpdateImp( TFLOAT a_fDeltaTime );                    // +0x00
	virtual ~AModelInstance2();                                       // +0x04
	virtual void OnLoaded( Toshi::TModelInstance* a_pModelInstance ); // +0x08
	virtual void OnUnloaded();                                        // +0x0C

	void SetTransform( const Toshi::TMatrix44& a_rMatrix );
	TNOINLINE void GetTransform( Toshi::TMatrix44& a_rOutMatrix ); // NOTE: retail calls it from its own file (the matrix copy is small)
	void SetPosition( const Toshi::TVector4& a_rPosition );
	void GetPosition( Toshi::TVector4& a_rOutPosition );

	void SetFlag( FLAG a_uiFlag, TBOOL a_bSet );
	void SetRenderLighting( TFLOAT a_fAmbientScale, TBOOL a_bUseEnvironmentScale );

	void                  Render();
	TBOOL                 Render( Toshi::TModelInstance* a_pObj );
	const Toshi::TSphere* GetBoundingSphere();
	// The bounding sphere moved by the model instance's matrix (xyz: the centre, w: the radius)
	void GetBoundingSphereWorld( Toshi::TVector4& a_rOutSphere );
	void UpdateAnimation( TFLOAT a_fDeltaTime );

	static void __stdcall RenderGeneric( Toshi::TModelInstance* a_pModelInstance, void* a_pUserData );

	TINT16 FindBone( const TCHAR* a_szBoneName );

	// The world transform of bone a_iBoneId (scaled by the instance's scale)
	void GetBoneTransformLocal( TINT a_iBoneId, Toshi::TMatrix44& a_rOutMatrix );
	void GetBoneTransform( TINT a_iBoneId, Toshi::TMatrix44& a_rOutMatrix );

	// $deBlob: FUNCTION 005786c0
	void SetScale( const Toshi::TVector4& a_rvScale )
	{
		TASSERT( a_rvScale.x > 0 && a_rvScale.x < 1000 );
		TASSERT( a_rvScale.y > 0 && a_rvScale.y < 1000 );
		TASSERT( a_rvScale.z > 0 && a_rvScale.z < 1000 );
		TASSERT( a_rvScale.w == 1.0f );
		m_vScale = a_rvScale;
	}

	// $deBlob: FUNCTION 00578800
	void SetScale( TFLOAT a_fScale )
	{
		TASSERT( a_fScale > 0 && a_fScale < 1000 );
		m_vScale.Set( a_fScale, a_fScale, a_fScale, 1.0f );
	}

	const Toshi::TVector4& GetScale() const { return m_vScale; }

	// The per mesh settings
	void SetMeshFlags( TINT a_iMesh, TUINT32 a_uiFlags, TBOOL a_bSet )
	{
		if ( !a_bSet ) m_aMeshSettings[ a_iMesh ].uiFlags &= ~a_uiFlags;
		else m_aMeshSettings[ a_iMesh ].uiFlags |= a_uiFlags;
	}
	TUINT32 GetMeshFlags( TINT a_iMesh ) const { return m_aMeshSettings[ a_iMesh ].uiFlags; }

private:
	struct MeshSettings;

public:
	// The settings of a mesh (MAX_MESH_PROPS)
	// $deBlob: FUNCTION 0054f070
	MeshSettings* GetMeshSettings( TUINT8 a_iMesh )
	{
		TASSERT( a_iMesh < MAX_MESH_SETTINGS );
		return &m_aMeshSettings[ a_iMesh ];
	}
	// Writes the low byte of the mesh's flags (ABlobAnimationComponent::Render saves and restores that byte only)
	void SetMeshFlagsLowByte( TINT a_iMesh, TUINT8 a_uiFlags ) { *TREINTERPRETCAST( TUINT8*, &m_aMeshSettings[ a_iMesh ].uiFlags ) = a_uiFlags; }
	void SetMeshColour( TINT a_iMesh, TINT a_iComponent, TUINT8 a_uiValue ) { m_aMeshSettings[ a_iMesh ].aColour[ a_iComponent ] = a_uiValue; }
	void SetMeshColour( TINT a_iMesh, TUINT32 a_uiColour ) { *TREINTERPRETCAST( TUINT32*, m_aMeshSettings[ a_iMesh ].aColour ) = a_uiColour; }

	// $deBlob: FUNCTION 00571fe0
	void UseLocalAmbientLight( TBOOL a_bUse ) { SetFlag( FLAG_LocalAmbient, a_bUse ); }
	// $deBlob: FUNCTION 00572000
	void SetAmbientColour( TUINT32 a_uiColour ) { m_uiAmbientColour = a_uiColour; }
	void UseLocalAmbientColour( TBOOL a_bUse );
	void    SetLocalAmbientColour( TUINT32 a_uiColour );
	TUINT32 GetLocalAmbientColour() const { return m_uiLocalAmbientColour; }

	// Rendered by its model (has a TModelInstance and is visible and renderable)
	TBOOL IsRenderable() const { return m_pModelInstance && ( m_uiFlags & FLAG_Visible ) && ( m_uiFlags & FLAG_Renderable ); }

	AModel2*               GetModel() const { return m_pModel; }
	Toshi::TModelInstance* GetModelInstance() const { return m_pModelInstance; }
	// Drawn instead of the model's materials when set (the APropWater transitions)
	void                  SetOverrideMaterial( Toshi::TMaterial* a_pMaterial ) { m_uiOverrideMaterial = TUINT32( TREINTERPRETCAST( TUINTPTR, a_pMaterial ) ); }
	AToshiAnimInterface2* GetAnimInterface() const { return m_pAnimInterface; }
	// $deBlob: FUNCTION 0075e4e0
	void*   GetSoundBreakpointComponent() const { return m_pSoundBreakpointComponent; }
	void*   GetCollisionInstance() const { return m_pCollisionInstance; }
	void    SetCollisionInstance( void* a_pCollisionInstance ) { m_pCollisionInstance = a_pCollisionInstance; }
	void    SetAmbientScale( TFLOAT a_fAmbientScale ) { m_fAmbientScale = a_fAmbientScale; }
	TUINT16 GetFlags() const { return m_uiFlags; }

private:
	TUINT32                m_uiAmbientColour;           // 0x0C
	TUINT32                m_uiLocalAmbientColour;      // 0x10
	AModel2*               m_pModel;                    // 0x14
	AToshiAnimInterface2*  m_pAnimInterface;            // 0x18
	Toshi::TModelInstance* m_pModelInstance;            // 0x1C
	Toshi::TVector4        m_vScale;                    // 0x20 the scale (SetScale)
	void*                  m_pCollisionInstance;        // 0x30 ACollisionInstance2
	void*                  m_pSoundBreakpointComponent; // 0x34 ASoundBreakpointComponent
	FLAG                   m_uiFlags;                   // 0x38
	TUINT32                m_uiUnk3C;                   // 0x3C
	TUINT32                m_uiUnk40;                   // 0x40
	TUINT32                m_uiOverrideMaterial;        // 0x44
	struct MeshSettings
	{
		TUINT8 aColour[ 4 ];                // 0x00 -1 by default
		TUINT8 uiFlags;                     // 0x04 2 by default, 1: the colour is used
		TUINT8 aPad[ 3 ];                   // 0x05
	} m_aMeshSettings[ MAX_MESH_SETTINGS ]; // 0x48 per mesh of the model
	TFLOAT m_fAmbientScale;                 // 0x70 AVisibleEntity's "Ambient Scale"
};                                          // 0x74
