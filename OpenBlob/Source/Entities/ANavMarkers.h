#pragma once
#include <Math/TVector4.h>

namespace Toshi
{
class TTRB;
}

//-----------------------------------------------------------------------------
// Purpose: the level's navigation markers (4 bytes), the "NavMarkers" symbol of the level's NavMarkers block.
// NOTE: the layout of the data isn't ported
//-----------------------------------------------------------------------------
class ANavMarkers
{
public:
	// A marker (0x20 bytes)
	struct Marker
	{
		Toshi::TVector4 m_vPosition;    // 0x00
		Marker*         m_apLinks[ 2 ]; // 0x10 the neighbouring markers of the path
		TUINT32         m_uiUnk18;      // 0x18
		TUINT32         m_uiFlags;      // 0x1C 1: water
	};

	// A nav set: a run of markers (8 bytes)
	struct NavSet
	{
		Marker* m_pMarkers;    // 0x00
		TINT    m_iNumMarkers; // 0x04
	};

	// The "NavMarkers" block (m_pData)
	struct Data
	{
		Marker* m_pMarkers;    // 0x00
		TINT    m_iNumMarkers; // 0x04
		NavSet* m_pNavSets;    // 0x08 indexed by the nav set IDs of ATriggerUtil (APropPlatform)
	};

public:
	ANavMarkers( Toshi::TTRB* a_pTRB );
	~ANavMarkers();

	static ANavMarkers* GetSingleton() { return ms_pSingleton; }

	Data* GetData() const { return TSTATICCAST( Data, m_pData ); }

private:
	static ANavMarkers* ms_pSingleton;

private:
	void* m_pData; // 0x00 the "NavMarkers" symbol
};
