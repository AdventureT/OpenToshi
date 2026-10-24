#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Math/TRandom.h>

//-----------------------------------------------------------------------------
// ARandom (0x98): the game's random number generator.
//-----------------------------------------------------------------------------
class ARandom
    : public Toshi::TObject
    , public Toshi::TSingleton<ARandom>
{
public:
	TDECLARE_CLASS( ARandom, Toshi::TObject );

public:
	ARandom();
	~ARandom();

	Toshi::TRandom& GetRandom() { return m_oRandom; }

private:
	Toshi::TRandom m_oRandom; // 0x04
};
