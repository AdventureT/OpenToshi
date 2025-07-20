#pragma once

class AMusicManager : public Toshi::TSingleton<AMusicManager>
{
	enum Tracks
	{
		AMUSICMANAGER_MAX_TRACKS = 8
	};

	struct BuildingColour
	{
		TINT m_iNumberOfColours; // 0x0
	};

	void Update(TFLOAT deltatime);
};
