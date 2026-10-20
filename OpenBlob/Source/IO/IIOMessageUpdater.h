#pragma once

class AIOEntity;

//-----------------------------------------------------------------------------
// Purpose: the base of the IO updaters, the actions that run over time: an AIOEntity keeps them in a list (+0x100) and
// ticks the active ones. The derived updaters (IOMoveUpdater, IOScaleUpdater) are created by AIOEntity's
// SetupUpdateOverTimeMessages.
//-----------------------------------------------------------------------------
class IIOMessageUpdater
{
public:
	IIOMessageUpdater()
	{
		m_bActive = TFALSE;
		m_fTime   = 0.0f;
		m_pOwner  = TNULL;
		m_pAction = TNULL;
		m_pInput  = TNULL;
		m_pNext   = TNULL;
	}

	// NOTE: the plain destructors of the six updaters (this and IOFade/IOMove/IORotate/IOScale/IOShakeUpdater) are
	// byte-identical (they all store this class's vtable) and unreferenced; assigned in the order of the deleting ones
	// $deBlob: FUNCTION 00786a80
	// $deBlob: FUNCTION 007868f0
	virtual ~IIOMessageUpdater() {} // +0x00

	// Ticks the updater while it's active (AIOEntity::Tick)
	virtual void Update( TFLOAT a_fDeltaTime ) = 0; // +0x04

	// $deBlob: FUNCTION 00786ba0
	virtual void Activate() { m_bActive = TTRUE; } // +0x08

	TBOOL              IsActive() const { return m_bActive; }
	IIOMessageUpdater* GetNext() const { return m_pNext; }

	// Set by SetupUpdateOverTimeMessages
	void SetOwner( AIOEntity* a_pOwner ) { m_pOwner = a_pOwner; }
	// $deBlob: FUNCTION 007caa90
	void SetAction( void* a_pAction ) { m_pAction = a_pAction; }
	void SetInput( void* a_pInput ) { m_pInput = a_pInput; }
	void SetNext( IIOMessageUpdater* a_pNext ) { m_pNext = a_pNext; }

	// Set by AIOEntity when the action starts (FadeMesh, Scale)
	void  SetTime( TFLOAT a_fTime ) { m_fTime = a_fTime; }
	void* GetAction() const { return m_pAction; }

	// AIOEntity's Reset and StopRotations
	void Stop()
	{
		m_bActive = TFALSE;
		m_fTime   = 0.0f;
	}

protected:
	// The parameters of the action (its +8)
	template <class T>
	T* GetParams() const { return *TREINTERPRETCAST( T**, TSTATICCAST( TUINT8, m_pAction ) + 8 ); }

	// Sends the owner the message a_iFinished (1..8) of the action's parameters once the update is done (the
	// messages 0xC..0xF and 0x4A..0x4D, TNULL as the other entity)
	void SendFinishedMessage( TINT a_iFinished );

protected:
	TBOOL              m_bActive; // 0x04
	TFLOAT             m_fTime;   // 0x08 the time of the update
	AIOEntity*         m_pOwner;  // 0x0C
	void*              m_pAction; // 0x10 the IO data's action
	void*              m_pInput;  // 0x14 the IO data's input of the action
	IIOMessageUpdater* m_pNext;   // 0x18 AIOEntity's list
}; // 0x1C
