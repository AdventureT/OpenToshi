#pragma once

//-----------------------------------------------------------------------------
// Purpose: the message an entity sends to another through ABaseEntity::HandleEntityIO. m_iData is message specific,
// often a pointer (an AAttackMessage for the attacks).
//-----------------------------------------------------------------------------
struct AEntityIOPacket
{
	enum MESSAGE : TINT
	{
		MESSAGE_NONE              = 0,
		MESSAGE_TRIGGER           = 1,
		MESSAGE_UNTOUCH           = 2, // the toucher left (sent to the touched entity)
		MESSAGE_PLAYERINTERACT    = 3, // switches, pickups
		MESSAGE_TOUCH             = 4, // sent to the touched entity
		MESSAGE_TOUCHEDBY         = 5, // the reply to the toucher
		MESSAGE_PLAYERATTACK      = 6, // m_iData: AAttackMessage*
		MESSAGE_SLAM              = 7, // the blob's Z-jump hit, m_iData: AAttackMessage*
		MESSAGE_GRAPPLESTRUGGLE   = 8,
		MESSAGE_GRAPPLERELEASE    = 9,
		MESSAGE_RECOIL            = 10, // m_iData: AAttackMessage* (ATTACK_SlamFailedRecoil)
		MESSAGE_ENEMYATTACK       = 11, // m_iData: AAttackMessage*
		MESSAGE_PROPATTACK        = 12, // m_iData: AAttackMessage*
		MESSAGE_STOP              = 13,
		MESSAGE_OPEN              = 14,
		MESSAGE_CLOSE             = 15,
		MESSAGE_DEATHZONE         = 16,
		MESSAGE_BOUNCE            = 17,
		MESSAGE_ENTER             = 18,
		MESSAGE_ENEMYKILLED       = 23, // sent by an enemy wrapper to its owner
		MESSAGE_ENEMYRELEASED     = 24, // sent by an enemy to its wrapper
		MESSAGE_PICKEDUP          = 25, // the reply to MESSAGE_PLAYERINTERACT of a pickup
		MESSAGE_THROW             = 26, // m_iData: APickupableComponent::ThrowData*
		MESSAGE_THROW2            = 27, // handled like MESSAGE_THROW
		MESSAGE_WIND              = 28,
		MESSAGE_UNBALANCE         = 30,
		MESSAGE_UNK31             = 31,
		MESSAGE_IOINPUT           = 33, // m_iData: TINT* (the AIOEntity input message)
		MESSAGE_RAMATTACK         = 34, // m_iData: AAttackMessage*
		MESSAGE_CHALLENGEINIT     = 35,
		MESSAGE_CHALLENGERESET    = 36,
		MESSAGE_CHALLENGECOMPLETE = 37,
		MESSAGE_CHALLENGESTART    = 38,
		MESSAGE_PROJECTILEHIT     = 39, // m_iData: AProjectile*
		MESSAGE_TOGGLE            = 40,
		MESSAGE_CULLED            = 41, // a pooled entity went out of sight
		MESSAGE_RECLAIM           = 42, // a suicider enemy is taken from its wrapper
		MESSAGE_ENABLE            = 43,
		MESSAGE_DESPAWN           = 44,
		MESSAGE_SKIP              = 45, // the cutscene was skipped
		MESSAGE_RESET             = 46,
	};

	MESSAGE m_eMessage; // 0x00
	TINT    m_iData;    // 0x04
};
