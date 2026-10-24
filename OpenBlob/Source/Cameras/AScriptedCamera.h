#pragma once
#include "ACamera.h"

class AModelInstance2;

namespace Toshi
{
class TAnimation;
}

//-----------------------------------------------------------------------------
// Purpose: a camera playing an animated camera from a TRB, e.g. the respawn camera of APlayerManager
// ("Blob_Respawn_Camera.trb"): the camera follows a bone of an animated model.
//-----------------------------------------------------------------------------
class AScriptedCamera : public ACamera
{
public:
	TDECLARE_CLASS( AScriptedCamera, ACamera );

public:
	AScriptedCamera();
	~AScriptedCamera();

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x0C

	// Loads the model a_szFileName; the camera follows a_szBone (and a_szBone2, if given, is the second camera bone)
	TBOOL InitialiseActor( const TCHAR* a_szFileName, const TCHAR* a_szBone, const TCHAR* a_szBone2 );
	void  DeinitialiseActor();

	// Plays the model's animation a_szName; returns whether it was started
	TBOOL  PlayAnimation( const TCHAR* a_szName );
	TBOOL  IsAnimating();
	TFLOAT GetAnimationLength();
	TFLOAT GetAnimationTime();
	void   StopAnimation();

	// Out: the world matrix of camera bone a_iBone (0 or 1), or identity if it doesn't exist; always returns TFALSE
	TBOOL GetBoneMatrix( Toshi::TMatrix44& a_rOutMatrix, TINT a_iBone );
	void  SetTransform( const Toshi::TMatrix44& a_rcMatrix );

	TFLOAT GetBlendOutStart() const { return m_fBlendOutStart; }
	TFLOAT GetBlendOutEnd() const { return m_fBlendOutEnd; }
	void   SetBlendOut( TFLOAT a_fStart, TFLOAT a_fEnd )
	{
		m_fBlendOutStart = a_fStart;
		m_fBlendOutEnd   = a_fEnd;
	}

private:
	AModelInstance2*   m_pModelInstance; // 0x104 the camera's animated model instance
	TBOOL              m_bPlaying;       // 0x108
	TINT16             m_aiBones[ 2 ];   // 0x10A -1 the camera bones
	TINT16             m_iSequence;      // 0x10E -1 the animation playing
	Toshi::TAnimation* m_pAnimation;     // 0x110
	TFLOAT             m_fBlendOutStart; // 0x114 0.0f fraction of the animation where ABlobFollowCamera starts blending out
	TFLOAT             m_fBlendOutEnd;   // 0x118 1.0f fraction of the animation where it's done
};
