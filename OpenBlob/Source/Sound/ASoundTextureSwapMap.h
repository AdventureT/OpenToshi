#pragma once
#include <Toshi/TSingleton.h>
#include <File/TTRB.h>
#include <Math/TVector4.h>

namespace Toshi
{
class TTexture;
}

//-----------------------------------------------------------------------------
// Purpose: the sounds played when the blob swaps a texture of the terrain (ATextureSwapInstance), loaded from
// "Data/Sound/TextureSwapSoundData/SoundTextureSwap.trb" (symbol "SoundTextureSwapMap"). Each entry maps a texture to
// an FMOD event (group and event names); created by AAppInitState.
// NOTE: the source file isn't known (no assert)
//-----------------------------------------------------------------------------
class ASoundTextureSwapMap
    : public Toshi::TSingleton<ASoundTextureSwapMap>
{
public:
	struct Entry
	{
		Toshi::TTexture* m_pTexture; // 0x00 resolved by ResolveTextures
		TINT16           m_iGroup;   // 0x04 index in m_ppStrings: the FMOD event group
		TINT16           m_iEvent;   // 0x06 index in m_ppStrings: the event
		TINT16           m_iTexture; // 0x08 index in m_ppStrings: the texture's name
		TINT16           m_iPad0A;   // 0x0A
	};

	struct Data
	{
		Entry*        m_pEntries;    // 0x00
		const TCHAR** m_ppStrings;   // 0x04
		TUINT16*      m_pGroups;     // 0x08 index in m_ppStrings of the sound groups to load
		TINT16        m_iNumEntries; // 0x0C
		TINT16        m_iPad0E;      // 0x0E
		TINT16        m_iNumGroups;  // 0x10
	};

public:
	ASoundTextureSwapMap();
	~ASoundTextureSwapMap();

	// Plays the sound of every entry of a_pTexture at a_rcPosition
	void PlayTextureSounds( Toshi::TTexture* a_pTexture, const Toshi::TVector4& a_rcPosition );

	// Plays the sound of the first entry of a_pTexture at a_rcPosition
	void PlayFirstTextureSound( Toshi::TTexture* a_pTexture, const Toshi::TVector4& a_rcPosition );

	// Adds the sound groups of the entries to ASoundLoadHelper
	void AddSoundGroupsToLoad();

	// Finds the texture of every entry
	void ResolveTextures();

private:
	void PlayEntrySound( Entry* a_pEntry, const Toshi::TVector4& a_rcPosition );

private:
	Data*       m_pData; // 0x00 TNULL if the TRB couldn't be loaded
	Toshi::TTRB m_oTRB;  // 0x04
};
