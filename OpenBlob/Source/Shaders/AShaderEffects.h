#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Platform/DX11/TWorldShader_DX11.h>

//-----------------------------------------------------------------------------
// Purpose: the game's shader effects: the copies of the materials of a model's meshes with one texture swapped (the
// texture overrides of ATextureSwapInstance). A copy fades its texture in through the world shader's crossfade table
// (TWorldShaderHAL::SetCrossfade) and is released when its time runs out.
// Created by ALoadLevelState's ctor and deleted by AInGameState::OnRemoval.
//-----------------------------------------------------------------------------
class AShaderEffects : public Toshi::TSingleton<AShaderEffects>
{
public:
	static TCONSTEXPR_VAR TINT MAX_CLONES = 16;

	// The callback of a copy, called by Update before the copy is released
	struct Clone;
	typedef void( __stdcall* t_CloneCallback )( Clone* a_pClone, void* a_pOwner );

	// A copy of a material with the new texture in stage m_iStage (retail's node: 0x24 bytes, in the active list while
	// it fades, in the free list otherwise). The copies of one override are chained by m_pNextCopy (+0x20).
	struct Clone : public Toshi::T2GenericDList::Node
	{
		TINT16                    m_iIndex;    // 0x08 the crossfade index and the material's user data index (UData_2)
		TINT16                    m_iStage;    // 0x0A the stage of the material the texture is in
		TFLOAT                    m_fTime;     // 0x0C 1.0 when added, Update takes 2 * dt off it
		Toshi::TWorldMaterialHAL* m_pMaterial; // 0x10 the copy of the material
		TUINT8                    m_bFlag;     // 0x14 the copy takes two passes (the lerp of a paint pass)
		t_CloneCallback           m_pCallback; // 0x18 called when the time is up
		void*                     m_pOwner;    // 0x1C the texture override the copy belongs to
		Clone*                    m_pNextCopy; // 0x20 the next copy of the same override
	};
	TSTATICASSERT( sizeof( Clone ) == 0x24 );

public:
	AShaderEffects();
	virtual ~AShaderEffects(); // +0x00

	// Fades the copies (a_fDelta in seconds).
	void Update( TFLOAT a_fDelta );

	// Releases every active copy.
	void ReleaseAll();

	// Takes a free copy for the material's stage a_iStage (the material's pass count must be 1, or 2 with a_bFlag) and
	// returns it, or TNULL when there is none or the material doesn't match.
	Clone* AddClone( Toshi::TWorldMaterialHAL* a_pMaterial, Toshi::TTexture* a_pTexture, TINT16 a_iStage, TBOOL a_bFlag, t_CloneCallback a_pCallback, void* a_pOwner );

	// Restores the material's texture stack and puts the copy back in the free list.
	void ReleaseClone( Clone* a_pClone, TINT a_iUnused ); // a_iUnused: retail pushes it, never reads it

private:
	Toshi::T2DList<Clone> m_oActiveClones; // 0x04 the copies that fade
	Toshi::T2DList<Clone> m_oFreeClones;   // 0x0C the copies that can be taken
	Clone*                m_paClones;      // 0x14 the pool (MAX_CLONES)
};
