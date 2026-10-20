#pragma once
#include "Entities/AInky_Baton/AInky_Baton.h"

//-----------------------------------------------------------------------------
// Purpose: the elite Inky soldier with the baton (Entities\AInky_Baton_Elite\AInky_Baton_Elite.cpp): an AInky_Baton
// whose type (m_uiType) is the colour of the elite baton. It reports the type + 0x10 (GetTargetColour), 4 (GetMaxLife) and plays its
// own sound on PlayDeathSound.
//-----------------------------------------------------------------------------
class AInky_Baton_Elite
    : public AInky_Baton
{
public:
	TDECLARE_CLASS( AInky_Baton_Elite, AInky_Baton );

public:
	AInky_Baton_Elite();
	virtual ~AInky_Baton_Elite() override; // +0x08

	virtual TINT GetTargetColour() override;                   // +0xB4
	virtual TINT GetMaxLife() override;                        // +0xB8
	virtual void SetCustomData( TINT a_iCustomData ) override; // +0x1A0
	virtual void PlayDeathSound() override;                    // +0x22C
};
