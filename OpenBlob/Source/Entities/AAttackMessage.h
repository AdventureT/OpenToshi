#pragma once
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// The data of an attack (sent as the m_iData of an AEntityIOPacket, e.g. MESSAGE_RECOIL to a blob that slammed a LeechBot
// without paint, messages 0xB/0xC/0x22 when an enemy hits it).
// NOTE: class
//-----------------------------------------------------------------------------
struct AAttackMessage
{
	enum ATTACKTYPE : TINT
	{
		ATTACK_PlayerSlamDirect = 3, // the blob slammed the target
		ATTACK_PlayerSlamSplash = 4, // the target is near a slam
		ATTACK_SlamFailedRecoil = 8,
	};

	// $deBlob: FUNCTION 007bffb0
	AAttackMessage( ATTACKTYPE a_eType, const Toshi::TVector4& a_rPosition, TUINT a_uiDamage, TUINT a_uiUnk18, const Toshi::TVector4& a_rDirection, TBOOL a_bUnk2C )
	    : m_eType( a_eType )
	    , m_vPosition( a_rPosition )
	    , m_uiDamage( a_uiDamage )
	    , m_uiUnk18( a_uiUnk18 )
	    , m_vDirection( a_rDirection )
	    , m_bUnk2C( a_bUnk2C )
	{}

	ATTACKTYPE      m_eType;      // 0x00
	Toshi::TVector4 m_vPosition;  // 0x04
	TUINT           m_uiDamage;   // 0x14
	TUINT           m_uiUnk18;    // 0x18
	Toshi::TVector4 m_vDirection; // 0x1C
	TBOOL           m_bUnk2C;     // 0x2C
};

TSTATICASSERT( sizeof( AAttackMessage ) == 0x30 );
