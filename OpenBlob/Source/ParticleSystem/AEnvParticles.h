#pragma once
#include "A2DHash.h"
#include "A2DHashItem.h"
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>

namespace Toshi
{
class TParticleEffect;
class TParticleEffectInstance;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: a placed environment particle effect of AEnvParticles: an item of its A2DHash at the translation of
// m_mTransform with m_fRadius. Its effect instance only exists while a camera is near.
//-----------------------------------------------------------------------------
class AEnvEmitter
    : public A2DHashItem
{
public:
	enum FLAGS : TUINT32
	{
		FLAGS_Unk1   = 1 << 0, // started into the second active list and before the others (queue m_apQueued1)
		FLAGS_Queued = 1 << 1, // in a start queue of this update
	};

public:
	// $deBlob: FUNCTION 005e9e50
	AEnvEmitter()
	    : m_pParticleEffectInstance( TNULL ), m_pEffect( TNULL )
	{}

	// $deBlob: FUNCTION 005e9f20
	// $deBlob: FUNCTION 005ea080
	~AEnvEmitter() override = default;

	// $deBlob: FUNCTION 005ea240
	const Toshi::TVector4* GetPosition() override { return &m_mTransform.GetTranslation(); }

	// $deBlob: FUNCTION 005ea260
	TFLOAT GetRadius() override { return m_fRadius; }

public:
	TUINT32                         m_uiFlags;                 // 0x14 FLAGS
	Toshi::TParticleEffectInstance* m_pParticleEffectInstance; // 0x18
	Toshi::TParticleEffect*         m_pEffect;                 // 0x1C
	TFLOAT                          m_fRadius;                 // 0x20
	Toshi::TMatrix44                m_mTransform;              // 0x24
};

//-----------------------------------------------------------------------------
// Purpose: the environment particle effects of the level (created by ALoadLevelState with 500 emitters, deleted by
// AInGameState::OnRemoval). The emitters are kept in an A2DHash; each update the ones within 250 of a used viewport's
// camera get an effect instance (at most MAX_ACTIVE_EMITTERS at a time), the others lose theirs.
//-----------------------------------------------------------------------------
class AEnvParticles
    : public Toshi::TSingleton<AEnvParticles>
{
public:
	// The number of effect instances that may run at once, also the size of the start queues
	static TINT s_iMaxActiveEmitters;

public:
	AEnvParticles( TINT a_iMaxEmitters );
	virtual ~AEnvParticles(); // +0x00

	// Nothing to load (the emitters are added by their owners)
	// $deBlob: FUNCTION 005e8d30
	void Load( const TCHAR* a_szFileName ) {} // DOES_NOTHING

	// $deBlob: FUNCTION 005e8980
	AEnvEmitter* GetEmitters() const { return m_paEmitters; }
	// $deBlob: FUNCTION 005e8d40
	TBOOL IsLoaded() const { return TTRUE; }

	// Starts the effects of the emitters near the cameras and stops the others (AInGameState::OnUpdate)
	void Update( TFLOAT a_fDeltaTime );

	// Adds an emitter of a_pEffect at a_rTransform; a_fRadius <= 0: the largest fixed radius of the effect's systems.
	// Returns TNULL when no emitter is free
	AEnvEmitter* AddEmitter( Toshi::TParticleEffect* a_pEffect, const Toshi::TMatrix44& a_rTransform, TFLOAT a_fRadius, TUINT32 a_uiFlags );

	// AddEmitter with the effect named a_szEffect (TNULL if there's none)
	AEnvEmitter* AddEmitter( const TCHAR* a_szEffect, const Toshi::TMatrix44& a_rTransform, TFLOAT a_fRadius, TUINT32 a_uiFlags );

	// Stops the emitter's effect and gives the emitter back to the free list
	void RemoveEmitter( AEnvEmitter* a_pEmitter, TBOOL a_bInstant );

	// Moves the emitter (and its effect instance) to a_rTransform
	void SetEmitterTransform( AEnvEmitter* a_pEmitter, const Toshi::TMatrix44& a_rTransform );

	// Stops the effects of all the emitters at once (the level restart); the emitters stay
	void Reset();

	// Removes all the emitters (the destructor)
	void RemoveAllEmitters();

private:
	// Starts the effect of the emitter and moves it to the list of a_pList
	static TBOOL __stdcall StartEmitter( AEnvEmitter* a_pEmitter, Toshi::T2GenericDList::Node* a_pList );

	// The finished callback of an emitter's effect instance: the emitter goes back to the inactive list
	static void OnEffectFinished( Toshi::TParticleEffectInstance* a_pEffectInstance, void* a_pUserData );

	// Stops the emitter's effect (if it has one) and moves it to the inactive list
	void StopEmitter( AEnvEmitter* a_pEmitter, TBOOL a_bInstant );

	// The callback of the A2DHash once its generation wraps: the stamps of the emitters are set to a_uiStamp
	static void SetEmitterStamps( TUINT32 a_uiStamp );

private:
	// The number of emitters with an effect instance
	static TINT sm_iNumActiveEmitters;
	// m_oInactive of the instance, used by OnEffectFinished
	static Toshi::T2GenericDList::Node* sm_pInactiveList;

private:
	TINT                        m_iMaxEmitters; // 0x04
	TINT                        m_iNumEmitters; // 0x08
	AEnvEmitter*                m_paEmitters;   // 0x0C
	Toshi::T2GenericDList::Node m_oActive;      // 0x10 with an effect instance, started from m_apQueued
	Toshi::T2GenericDList::Node m_oActive1;     // 0x18 with an effect instance, started from m_apQueued1
	Toshi::T2GenericDList::Node m_oInactive;    // 0x20 without an effect instance
	Toshi::T2GenericDList::Node m_oFree;        // 0x28
	A2DHash*                    m_pHash;        // 0x30
	AEnvEmitter**               m_apQueued;     // 0x34 the emitters to start (without FLAGS_Unk1)
	AEnvEmitter**               m_apQueued1;    // 0x38 the emitters to start (with FLAGS_Unk1)
};
