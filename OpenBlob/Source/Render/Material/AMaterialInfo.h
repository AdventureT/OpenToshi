#pragma once
#include <Toshi/T2DList.h>
#include <Render/TModel.h>

class AMaterialInfoPool;
class AUVAnimation;

namespace Toshi
{
class T2Material;
class TMaterial;
class TWorldMaterialHAL;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the UV animations of a material (meta material T2Material), one per pass (Source\Render\Material\
// AMaterialInfo.cpp). A TMaterial points to it with its UV animation (TMaterial::SetUVAnimation).
//-----------------------------------------------------------------------------
class AMaterialInfo
    : public Toshi::T2DList<AMaterialInfo>::Node
{
public:
	static TCONSTEXPR_VAR TINT MAX_PASSES = 4;

	friend class AMaterialInfoPool;
	friend class AMaterialManager;

public:
	AMaterialInfo();
	~AMaterialInfo() = default;

	AMaterialInfoPool* GetOwnerPool() const { return m_pOwnerPool; }
	// $deBlob: FUNCTION 0079a4e0
	Toshi::T2Material* GetMaterial() const { return m_pMaterial; }
	AUVAnimation*      GetUVAnimation( TINT a_iPass ) const;

	// Copies the textures and stage settings of a_pSource to a_pDest (same class)
	static void CopyMaterial( Toshi::TMaterial* a_pSource, Toshi::TMaterial* a_pDest );

	// Swaps the textures, UV streams, offsets, blend types and UV modes of the first two passes (the light map
	// becomes the first pass)
	static void SwapLightMapPasses( Toshi::TWorldMaterialHAL* a_pMat );

	// TRUE if the first mesh of the model's LOD 0 is of a_pClass (used by ATerrain2Interface for the world meshes of
	// the low LOD models)
	static TBOOL IsModelFirstMeshClass( const Toshi::T2ModelPtr& a_rModel, const Toshi::TClass* a_pClass );

private:
	AMaterialInfoPool* m_pOwnerPool;                   // 0x08
	TUINT32            m_uiUnk0C;                      // 0x0C
	AUVAnimation*      m_apUVAnimations[ MAX_PASSES ]; // 0x10
	Toshi::T2Material* m_pMaterial;                    // 0x20
};                                                     // 0x24

//-----------------------------------------------------------------------------
// Purpose: a fixed number of AMaterialInfos
//-----------------------------------------------------------------------------
class AMaterialInfoPool
{
public:
	friend class AMaterialManager;

public:
	AMaterialInfoPool( TINT a_iSize, const TCHAR* a_szName );
	~AMaterialInfoPool();

	// The name is the cell's (ACellBucket::OnLoaded)
	void SetName( const TCHAR* a_szName ) { m_szName = a_szName; }

	TBOOL IsUsedEmpty() const { return m_lUsed.IsEmpty(); }

	// Moves a free info to the used list
	AMaterialInfo* Alloc();

	// Gives a_rMatInfo back to its pool
	static void Free( AMaterialInfo& a_rMatInfo );

private:
	const TCHAR*                  m_szName; // 0x00
	TINT                          m_iSize;  // 0x04
	AMaterialInfo*                m_pInfos; // 0x08
	Toshi::T2DList<AMaterialInfo> m_lUsed;  // 0x0C
	Toshi::T2DList<AMaterialInfo> m_lFree;  // 0x14
};                                          // 0x1C
