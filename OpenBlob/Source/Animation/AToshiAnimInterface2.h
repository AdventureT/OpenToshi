#pragma once
#include <Render/TSkeleton.h>

class AAnimationPose;

//-----------------------------------------------------------------------------
// Purpose: plays the animations of a model instance's skeleton (4 bytes).
// Created by AModelInstance2 when asked for; the skeleton instance is set by AModelInstance2::OnLoaded/OnUnloaded.
//-----------------------------------------------------------------------------
class AToshiAnimInterface2
{
public:
	AToshiAnimInterface2();
	~AToshiAnimInterface2();

	void               SetPose( AAnimationPose* a_pPose, TFLOAT a_fBlendInSpeed, TFLOAT a_fSpeedMult );
	void               SetInvalidPose();
	TFLOAT             GetPoseLength( AAnimationPose* a_pPose );
	Toshi::TAnimation* AddAnim( TINT16 a_iSequence, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed );
	void               StopAnim( TINT16 a_iSequence, TFLOAT a_fBlendOutSpeed );
	void               StopAllAnims( TFLOAT a_fBlendOutSpeed );
	TBOOL              IsAnimationFinished( TINT16 a_iSequence );
	TFLOAT             GetAnimationLength( TINT16 a_iSequence );
	TFLOAT             GetAnimationTotalTime( TINT16 a_iSequence );
	TFLOAT             GetAnimationSeqTime( TINT16 a_iSequence );
	TINT16             GetAnimID( const TCHAR* a_szName );

	void SetSkeletonInstance( Toshi::TSkeletonInstance* a_pSkeletonInstance );

	// $deBlob: FUNCTION 00574220
	Toshi::TSkeletonInstance* GetSkeletonInstance() const { return m_pSkeletonInstance; }

private:
	Toshi::TSkeletonInstance* m_pSkeletonInstance; // 0x00
};
