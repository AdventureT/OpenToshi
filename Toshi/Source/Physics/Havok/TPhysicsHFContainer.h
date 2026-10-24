#pragma once
#include "Math/TMatrix44.h"
#include "Math/TVector4.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The data of a height field made from existing samples
//-----------------------------------------------------------------------------
struct TPhysicsHFDesc
{
	TINT     m_x;          // 0x00
	TINT     m_y;          // 0x04
	TFLOAT   m_fMinHeight; // 0x08
	TFLOAT   m_fMaxHeight; // 0x0c
	TUINT16* m_data;       // 0x10 not owned by the container
};

//-----------------------------------------------------------------------------
// Purpose: the samples of a height field body (TPhysicsObjectContainer shape type 8, 0xc4 bytes). Each sample is a
// TUINT16: the low 13 bits are the height (0..4096 maps to m_fMinHeight..m_fMaxHeight), the high 3 bits the index
// of one of 8 materials. TPhysicsManager::AddToSimulation builds a TPhysicsHeightFieldShape on it. NOTE: nothing in
// retail creates one.
//-----------------------------------------------------------------------------
class TPhysicsHFContainer
{
public:
	// The 3 values of a material; their meaning isn't known (defaults 1.0, 0.0, 0.1)
	struct Material
	{
		TFLOAT m_fUnk0; // 0x00
		TFLOAT m_fUnk4; // 0x04
		TFLOAT m_fUnk8; // 0x08
	};

	static TCONSTEXPR_VAR TINT MAX_MATERIALS = 8;

public:
	TPhysicsHFContainer();
	~TPhysicsHFContainer();

	// Allocates/frees a container on the physics heap
	static TPhysicsHFContainer* Create();
	static void                 Destroy( TPhysicsHFContainer* a_pContainer );

	// Allocates a_x * a_y samples (all 0) for heights between a_fMinHeight and a_fMaxHeight over a_fSizeX * a_fSizeY
	void CreateData( TINT a_x, TINT a_y, TFLOAT a_fMinHeight, TFLOAT a_fMaxHeight, TFLOAT a_fSizeX, TFLOAT a_fSizeY );

	void AssertData() const;

	// Uses the samples of a_rDesc (not owned), one unit per sample
	void Set( const TPhysicsHFDesc& a_rDesc );

	void   SetHeight( TINT a_x, TINT a_y, TFLOAT a_fHeight );
	TFLOAT GetHeight( TINT a_x, TINT a_y ) const;

	void SetMaterialIndex( TINT a_x, TINT a_y, TINT a_iIndex );
	TINT GetMaterialIndex( TINT a_x, TINT a_y ) const;

	void SetMaterial( TINT a_index, TFLOAT a_fUnk0, TFLOAT a_fUnk4, TFLOAT a_fUnk8 );
	void GetMaterial( TINT a_index, TFLOAT& a_rfUnk0, TFLOAT& a_rfUnk4, TFLOAT& a_rfUnk8 ) const;

	TVector4 GetDownVector() const;

	// Same as GetHeight/GetMaterial with the y axis flipped (the Havok shape's z)
	TFLOAT GetHeightAt( TINT a_x, TINT a_z ) const;
	void   GetMaterialAt( TINT a_x, TINT a_z, TFLOAT& a_rfUnk0, TFLOAT& a_rfUnk4 ) const;

	// $deBlob: FUNCTION 00734990
	TINT GetX() const { return m_x; }

	// $deBlob: FUNCTION 007349a0
	TINT GetY() const { return m_y; }

public:
	TMatrix44 m_oInvTransform;               // 0x00 inverse of the body's matrix (TPhysicsManager::AddToSimulation)
	TUINT16*  m_data;                        // 0x40
	TINT      m_x;                           // 0x44
	TINT      m_y;                           // 0x48
	TFLOAT    m_fMinHeight;                  // 0x4c
	TFLOAT    m_fMaxHeight;                  // 0x50
	TFLOAT    m_fHeightRange;                // 0x54 m_fMaxHeight - m_fMinHeight, at least 0.001
	TFLOAT    m_fSizeX;                      // 0x58
	TFLOAT    m_fSizeY;                      // 0x5c
	Material  m_aMaterials[ MAX_MATERIALS ]; // 0x60
	TBOOL     m_bExternalData;               // 0xc0 m_data isn't owned
};

TOSHI_NAMESPACE_END
