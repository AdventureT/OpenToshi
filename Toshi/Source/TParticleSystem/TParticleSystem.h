#pragma once
#include "Toshi/TNodeList.h"
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"
#include "Math/TPlane.h"
#include "TParticles.h"

// The TOSHI particle system.

TOSHI_NAMESPACE_START

class TTRB;
class TParticleEffectManager;
class TParticleMaterialManager;
class TParticleMesh;
class TMesh;
class TSysMesh;
class TTestParticles;
class TGenericParticles;
class TParticleMaterial;

class TParticleSystem
{
public:
	enum Flags : TUINT
	{
		Flags_OwnsTRB = 1, // the TRB passed to LoadTRB is destroyed with the system
	};

	friend class TParticles;
	friend class TGenericParticles;
	friend class TTestParticles;

	enum PARTICLES
	{
		PARTICLES_TEST,
		PARTICLES_GENERIC,
		PARTICLES_NUMOF
	};

	struct ACreateData
	{
		ACreateData()
		{
			m_iMaxFloats          = 16000;
			m_iMaxVec4s           = 16000;
			m_iMaxQuaternions     = 4096;
			m_iMaxUChars          = 16000;
			m_iMaxUInts           = 12096;
			m_iMaxEffectInstances = 128;
			m_iNumAppendedTRBs    = 6;
			m_iMaxNumMeshes       = 0;
			m_iMaxParticles       = 0;
			m_iNumSystems         = 50;
		}

		// The sizes of the TSharedBuffer pools Create makes (elements, 32 per buffer)
		TINT m_iMaxFloats;          // 0x00
		TINT m_iMaxVec4s;           // 0x04
		TINT m_iMaxQuaternions;     // 0x08
		TINT m_iMaxUChars;          // 0x0C
		TINT m_iMaxUInts;           // 0x10
		TINT m_iMaxEffectInstances; // 0x14
		TINT m_iMaxNumMeshes;       // 0x18
		TINT m_iMaxParticles;       // 0x1C (the vertices of all the particles, SetDetail overrides it)
		TINT m_iNumSystems;         // 0x20
		TINT m_iNumAppendedTRBs;    // 0x24
	};

	struct AMeshNode : public TNodeList<AMeshNode>::TNode
	{
		~AMeshNode();

		TMatrix44 m_oModelView; // 0x0C
		TSysMesh* m_pMesh;      // 0x4C
	};

	struct TParticlesNode : public TNodeList<TParticlesNode>::TNode
	{
		~TParticlesNode();

		TParticles* m_pParticles; // 0x0C
		PARTICLES   m_eType;      // 0x10
	};

public:
	TParticleSystem( const ACreateData& a_rCreateData );
	~TParticleSystem();

	void LoadTRB( const TCHAR* a_szName );
	void LoadTRB( TTRB* a_pTRB, const TCHAR* a_szName, TBOOL a_bOwnsTRB );
	TINT AppendTRB( TTRB* a_pTRB );
	void RemoveTRB( TINT a_iIndex );
	void Destroy();
	void SetDetail( TINT a_iDetail, const ACreateData* a_pCreateData );
	void Render();
	void Update( TFLOAT a_fDeltaTime );

	TParticles* CreateParticles( PARTICLES a_eType );
	void        StopAllParticles();
	void        StopParticles( TParticles* a_pParticles );
	void        StopAllGenericParticles( void* a_pSettings );

	TMatrix44& GetModelView();
	void       SetModelView( const TMatrix44& a_rModelView );
	TParticleMesh* GetMesh( TUINT a_uiNumVerts, TUINT a_uiMinVerts, TParticleMaterial* a_pMaterial );
	void           ResizeLockedMeshes();

	// Statistics and device helpers
	TINT  GetMaxQuads() const;
	TINT  GetNumGenericSystems() const;
	TINT  GetNumRenderingParticles() const;
	TINT  GetNumParticles() const;
	TINT  GetNumUsedSystems() const;
	TINT  GetNumUsedGenericSystems() const;
	TBOOL ReleaseMeshes();
	TBOOL RecreateMeshes();

	void SetMaterialsDirty() { m_bMaterialsDirty = TTRUE; }

	// $deBlob: FUNCTION 00719ea0
	TPlane* GetWorldPlanes() { return m_aWorldPlanes; }
	// $deBlob: FUNCTION 00719eb0
	TINT GetNumWorldPlanes() const { return m_iNumWorldPlanes; }

	TParticleEffectManager*   GetEffectManager() const { return m_pEffectManager; }
	TParticleMaterialManager* GetMaterialManager() const { return m_pMaterialManager; }

	static TParticleSystem* GetSingleton() { return s_pSingleton; }

private:
	void Create( const ACreateData& a_rCreateData );
	void GetParticleOffsets();
	void CreateMeshes();
	void DestroyMeshes();

public:
	static TParticleSystem* s_pSingleton;

	// The create data the game uses
	static ACreateData s_oCreateData;

	// NOTE: retail also stores the system in a second static right before Create
	static TParticleSystem* s_pCreatingSystem;

private:
	TTRB*                       m_pTRB;              // 0x00
	TParticleEffectManager*     m_pEffectManager;    // 0x04
	TParticleMaterialManager*   m_pMaterialManager;  // 0x08
	TNodeList<AMeshNode>        m_oUsedMeshes;       // 0x0C
	TNodeList<AMeshNode>        m_oFreeMeshes;       // 0x1C
	TNodeList<AMeshNode>        m_oRenderMeshes;     // 0x2C
	TNodeList<TParticlesNode>   m_aFreeParticles[ PARTICLES_NUMOF ]; // 0x3C
	TNodeList<TParticlesNode>   m_oUsedParticles;    // 0x5C
	TTestParticles*             m_pTestParticles;    // 0x6C
	TGenericParticles*          m_pGenericParticles; // 0x70
	TParticlesNode*             m_pAllocedParticleNodes; // 0x74
	TPlane                      m_aWorldPlanes[ 6 ]; // 0x78
	TINT                        m_iNumWorldPlanes;   // 0xD8
	TVector4                    m_aOffsets[ 4 ];     // 0xDC
	TVector4                    m_vRight;            // 0x11C
	TVector4                    m_vUp;               // 0x12C
	TVector4                    m_vForward;          // 0x13C
	TParticleMesh*              m_pLockedMeshes;     // 0x14C
	TINT                        m_iDetail;           // 0x150
	TFLOAT                      m_fLODStart;         // 0x154
	TINT                        m_iMaxParticles;     // 0x158
	TINT                        m_iMaxNumMeshes;     // 0x15C
	TINT                        m_aiNumSystems[ PARTICLES_NUMOF ]; // 0x160 (the particles of each type)
	TBOOL                       m_bUpdated;          // 0x168
	TINT                        m_iUnk16C;           // 0x16C
	TINT                        m_iUnk170;           // 0x170
	TTRB**                      m_ppAppendedTRBs;    // 0x174
	TINT                        m_iNumAppendedTRBs;  // 0x178
	TBOOL                       m_bMaterialsDirty;   // 0x17C
	TUINT                       m_uiFlags;           // 0x180
};

TSTATICASSERT( sizeof( TParticleSystem ) == 0x184 );

TOSHI_NAMESPACE_END
