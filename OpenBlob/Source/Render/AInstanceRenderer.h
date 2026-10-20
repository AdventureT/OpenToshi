#pragma once
#include <Math/TMatrix44.h>

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: draws many copies of a few small models (pickups, shadows) in batches. A model is registered by name (one
// AModelInstance2 per name, reference counted), its copies are submitted each frame with a rotation, a position, an
// alpha and an ambient colour, and Render draws them after the terrain (ARenderer::MainScene). Created by ARenderer.
//-----------------------------------------------------------------------------
class AInstanceRenderer
{
public:
	static TCONSTEXPR_VAR TINT MAX_MODELS       = 24;
	static TCONSTEXPR_VAR TINT MAX_NAME_LEN     = 64;
	static TCONSTEXPR_VAR TINT NUM_SMALLBATCHES = 30; // 8 copies
	static TCONSTEXPR_VAR TINT NUM_BIGBATCHES   = 8;  // 64 copies
	static TCONSTEXPR_VAR TINT NUM_BATCHES      = NUM_SMALLBATCHES + NUM_BIGBATCHES;

	// The submitted copies of a model (4 parallel arrays)
	struct Batch
	{
		const Toshi::TMatrix44** m_ppRotations; // 0x00 the rotation is read from the first 3 rows
		const Toshi::TVector4**  m_ppPositions; // 0x04
		TFLOAT*                  m_pAlphas;     // 0x08
		TUINT32*                 m_pColours;    // 0x0C local ambient colours
		TBOOL                    m_bUsed;       // 0x10
		TBOOL                    m_bBig;        // 0x11
	};                                          // 0x14

	struct Model
	{
		AModelInstance2* m_pInstance;              // 0x00
		TUINT16          registerCount;            // 0x04
		TUINT8           m_uiNumSubmitted;         // 0x06
		TUINT8           m_uiCapacity;             // 0x07 8 or 64
		Batch*           m_pBatch;                 // 0x08
		TCHAR            m_szName[ MAX_NAME_LEN ]; // 0x0C
	};                                             // 0x4C

public:
	AInstanceRenderer();
	~AInstanceRenderer();

	// The AModelInstance2 of a registered model (TNULL if a_szName isn't registered)
	AModelInstance2* FindInstance( const TCHAR* a_szName );

	// Registers a_szName (or adds a reference to it) and returns its index
	TINT8 RegisterModel( const TCHAR* a_szName );
	void  UnregisterModel( TINT8 a_Index );

	void SetupMaterials( TINT8 a_Index );

	// Adds a copy of model a_GFXIndex to this frame
	void SubmitForBatchRender( TINT8 a_GFXIndex, const Toshi::TMatrix44* a_pRotation, const Toshi::TVector4* a_pPosition, TFLOAT a_fAlpha, TUINT32 a_uiColour );

	void Render();

	// Forgets the copies submitted this frame
	void ClearSubmitted();

	static AInstanceRenderer* GetInstance() { return ms_Instance; }

private:
	Batch* AllocBatch( TINT a_iFirst, TINT a_iLast );
	// A small (a_iType 0) or big (1) batch, TNULL if none is free or for another type
	Batch* AllocBatch( TINT a_iType );
	void   FreeBatch( Batch* a_pBatch, TINT a_iCount );

private:
	static AInstanceRenderer* ms_Instance;

private:
	Model m_Models[ MAX_MODELS ];    // 0x000
	TINT8 m_iNumModels;              // 0x720
	Batch m_aBatches[ NUM_BATCHES ]; // 0x724
};
