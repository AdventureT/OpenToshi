#include "pch.h"
#include "ASSCameraFunction.h"
#include "ACurrentLevelInformation.h"

#include <File/TTRB.h>
#include <Math/TMath.h>
#include <Math/TMatrix44.h>
#include <Math/TPlane.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void ASSCameraFunction::SetFromKeyFrame( const KeyFrame& a_rKeyFrame, TVector4& a_rvPos, TQuaternion& a_rqOrient, TFLOAT& a_rfFOV )
{
	a_rvPos    = a_rKeyFrame.m_vPos;
	a_rqOrient = a_rKeyFrame.m_qOrient;
	a_rfFOV    = a_rKeyFrame.m_fFOV;
}

// $deBlob: FUNCTION 005949d0
void ASSCameraFunction::CatmullRom( TVector4& a_rvOut, const TVector4& a_rvP0, const TVector4& a_rvP1, const TVector4& a_rvP2, const TVector4& a_rvP3, TFLOAT a_fT )
{
	TVector4 v3P2, v3P1;
	v3P2.Multiply( a_rvP2, 3.0f );
	v3P1.Multiply( a_rvP1, 3.0f );
	TVector4 vA = ( a_rvP3 + ( v3P1 - v3P2 ) ) - a_rvP0;

	TVector4 vCubic;
	vCubic.Multiply( vA, a_fT * a_fT * a_fT );

	TVector4 v4P2, v5P1, v2P0;
	v4P2.Multiply( a_rvP2, 4.0f );
	v5P1.Multiply( a_rvP1, 5.0f );
	v2P0.Multiply( a_rvP0, 2.0f );
	TVector4 vB = ( v4P2 + ( v2P0 - v5P1 ) ) - a_rvP3;

	TVector4 vSquare;
	vSquare.Multiply( vB, a_fT * a_fT );

	TVector4 vC = a_rvP2 - a_rvP0;
	TVector4 vLinear;
	vLinear.Multiply( vC, a_fT );

	TVector4 v2P1;
	v2P1.Multiply( a_rvP1, 2.0f );

	TVector4 vSum = vCubic + vSquare + vLinear + v2P1;
	vSum.w        = v2P1.w;

	a_rvOut.Multiply( vSum, 0.5f );
}

// $deBlob: FUNCTION 00594e10
void ASSCameraFunction::Load()
{
	m_pHeader  = TNULL;
	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_SSCamera );
	m_pHeader  = TSTATICCAST( Header, pTRB->GetSymbolAddress( "SSCamHeader" ) );
}

// $deBlob: FUNCTION 00594e50
ASSCameraFunction::Camera* ASSCameraFunction::FindCamera( TINT a_iID ) const
{
	if ( m_pHeader == TNULL || a_iID < 0 )
		return TNULL;

	for ( TINT i = m_pHeader->m_iNumCameras; i != 0; )
	{
		i--;
		if ( m_pHeader->m_pCameras[ i ].m_iID == a_iID )
			return &m_pHeader->m_pCameras[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 00596a70
void ASSCameraFunction::InterpolateOrientation( const Camera* a_pCam, const TVector4& a_rvPos, TQuaternion& a_rqOrient, TFLOAT a_fT, TINT a_iFrame0, TINT a_iFrame1, TINT a_iFrame2, TINT a_iFrame3 )
{
	const TINT aFrames[ 4 ] = { a_iFrame0, a_iFrame1, a_iFrame2, a_iFrame3 };
	TVector4   avPoints[ 4 ];
	TMatrix44  mOrient;

	// Points above the key frames
	for ( TINT i = 0; i < 4; i++ )
	{
		const KeyFrame& rKeyFrame = a_pCam->m_pKeyFrames[ aFrames[ i ] ];
		avPoints[ i ]             = rKeyFrame.m_vPos;

		mOrient.SetFromQuaternion( rKeyFrame.m_qOrient );
		TVector4 vUpDir = mOrient.AsBasisVector4( BASISVECTOR_UP );
		TASSERT( vUpDir.IsNormalised() );

		TVector4 vOffset;
		vOffset.Multiply( vUpDir, 5.0f );
		avPoints[ i ].x += vOffset.x;
		avPoints[ i ].y += vOffset.y;
		avPoints[ i ].z += vOffset.z;
	}

	TVector4 vSpline;
	CatmullRom( vSpline, avPoints[ 0 ], avPoints[ 1 ], avPoints[ 2 ], avPoints[ 3 ], a_fT );

	TVector4 vUp = vSpline - a_rvPos;
	vUp.Normalise();

	// Points in front of the key frames
	for ( TINT i = 0; i < 4; i++ )
	{
		const KeyFrame& rKeyFrame = a_pCam->m_pKeyFrames[ aFrames[ i ] ];
		avPoints[ i ]             = rKeyFrame.m_vPos;

		mOrient.SetFromQuaternion( rKeyFrame.m_qOrient );
		TVector4 vLookDir = mOrient.AsBasisVector4( BASISVECTOR_FORWARD );
		TASSERT( vLookDir.IsNormalised() );

		avPoints[ i ].x += vLookDir.x;
		avPoints[ i ].y += vLookDir.y;
		avPoints[ i ].z += vLookDir.z;
	}

	CatmullRom( vSpline, avPoints[ 0 ], avPoints[ 1 ], avPoints[ 2 ], avPoints[ 3 ], a_fT );

	TVector4 vLook = vSpline - a_rvPos;
	vLook.Normalise();

	TVector4 vRight;
	vRight.CrossProduct( vUp, vLook );
	vRight.Normalise();
	vLook.CrossProduct( vRight, vUp );
	vLook.Normalise();

	mOrient.m_f41 = TMatrix44::IDENTITY.m_f41;
	mOrient.m_f42 = TMatrix44::IDENTITY.m_f42;
	mOrient.m_f43 = TMatrix44::IDENTITY.m_f43;
	mOrient.m_f44 = TMatrix44::IDENTITY.m_f44;
	mOrient.m_f11 = vRight.x;
	mOrient.m_f12 = vRight.y;
	mOrient.m_f13 = vRight.z;
	mOrient.m_f14 = 0.0f;
	mOrient.m_f21 = vUp.x;
	mOrient.m_f22 = vUp.y;
	mOrient.m_f23 = vUp.z;
	mOrient.m_f24 = 0.0f;
	mOrient.m_f31 = vLook.x;
	mOrient.m_f32 = vLook.y;
	mOrient.m_f33 = vLook.z;
	mOrient.m_f34 = 0.0f;
	TMatrix44::MatToQuat( a_rqOrient, mOrient );
}

// $deBlob: FUNCTION 00596820
void ASSCameraFunction::InterpolateKeyFrames( const Camera* a_pCam, TVector4& a_rvPos, TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TINT a_iFrame1, TINT a_iFrame2, TBOOL a_bLooped )
{
	TASSERT( a_pCam->m_iCount > 0 );

	TINT iFrame1 = a_iFrame1;
	TINT iFrame2 = a_iFrame2;

	if ( iFrame2 < iFrame1 )
	{
		iFrame1 = a_iFrame2;
		iFrame2 = a_iFrame1;
	}

	TINT iPrev;
	TINT iNext;

	if ( !a_bLooped )
	{
		TASSERT( ( iFrame2 - iFrame1 ) == 1 );
		iPrev = iFrame1 - 1;
		TMath::Clip( iPrev, 0, a_pCam->m_iCount - 1 );
		iNext = iFrame2 + 1;
		TMath::Clip( iNext, 0, a_pCam->m_iCount - 1 );
	}
	else
	{
		TINT iLast = iFrame2;

		if ( iFrame1 == 0 && iFrame2 == a_pCam->m_iCount - 1 )
		{
			// Between the last and the first key frame
			iLast   = 0;
			iFrame1 = iFrame2;
		}

		iPrev = iFrame1 - 1;
		iNext = iLast + 1;

		if ( iPrev < 0 )
			iPrev = a_pCam->m_iCount - 1;

		iFrame2 = iLast;

		if ( iNext >= a_pCam->m_iCount )
			iNext = 0;
	}

	const KeyFrame* pKeyFrames = a_pCam->m_pKeyFrames;
	TFLOAT          fDist1     = TVector4::Distance( pKeyFrames[ iFrame1 ].m_vPos, a_rvPos );
	TFLOAT          fDist2     = TVector4::Distance( pKeyFrames[ iFrame1 ].m_vPos, pKeyFrames[ iFrame2 ].m_vPos );
	TASSERT( fDist1 <= fDist2 );

	TFLOAT fT = 1.0f;

	if ( 0.0f < fDist2 )
	{
		fT = fDist1 / fDist2;
		TMath::Clip( fT, 0.0f, 1.0f );
	}

	CatmullRom( a_rvPos, pKeyFrames[ iPrev ].m_vPos, pKeyFrames[ iFrame1 ].m_vPos, pKeyFrames[ iFrame2 ].m_vPos, pKeyFrames[ iNext ].m_vPos, fT );
	InterpolateOrientation( a_pCam, a_rvPos, a_rqOrient, fT, iPrev, iFrame1, iFrame2, iNext );

	pKeyFrames = a_pCam->m_pKeyFrames;
	a_rfFOV    = ( pKeyFrames[ iFrame2 ].m_fFOV - pKeyFrames[ iFrame1 ].m_fFOV ) * fT + pKeyFrames[ iFrame1 ].m_fFOV;
}

// $deBlob: FUNCTION 00595bc0
void ASSCameraFunction::InterpolateAlongLine( const Camera* a_pCam, const TVector4& a_rvLineStart, const TVector4& a_rvLineEnd, const TVector4& a_rvFollowPos, const TVector4& a_rvViewPos, TVector4& a_rvPos, TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TFLOAT a_fLineRadius, TBOOL a_bLooped )
{
	TASSERT( a_pCam->m_iCount > 0 );

	TINT iCount = a_pCam->m_iCount;

	if ( iCount == 1 )
	{
		SetFromKeyFrame( a_pCam->m_pKeyFrames[ 0 ], a_rvPos, a_rqOrient, a_rfFOV );
		return;
	}

	TVector4 vStart  = a_rvLineStart;
	TVector4 vEnd    = a_rvLineStart;
	TVector4 vFollow = a_rvFollowPos;

	TVector4 vDir = a_rvLineEnd - a_rvLineStart;
	vDir.w        = a_rvLineEnd.w;
	vDir.Normalise();

	TVector4 vOffset;
	vOffset.Multiply( vDir, 10000.0f );
	vStart -= vOffset;
	vOffset.Multiply( vDir, 10000.0f );
	vEnd += vOffset;

	TFLOAT   fDist    = TMath::ClippedClosestPointOnLine( vFollow, vStart, vEnd );
	TVector4 vClosest = vStart;
	vOffset.Multiply( vDir, fDist );
	vClosest += vOffset;

	TFLOAT fDistSq = TVector4::DistanceSq( vClosest, vFollow );

	if ( fDistSq < a_fLineRadius * a_fLineRadius )
	{
		// Too close to the line: use the view position if it's further away
		TVector4 vViewClosest = vStart;
		TFLOAT   fViewDist    = TMath::ClippedClosestPointOnLine( a_rvViewPos, vStart, vEnd );
		vOffset.Multiply( vDir, fViewDist );
		vViewClosest += vOffset;

		if ( fDistSq < TVector4::DistanceSq( vViewClosest, a_rvViewPos ) )
		{
			vClosest = vViewClosest;
			vFollow  = a_rvViewPos;
		}
	}

	TPlane oPlane;
	oPlane.Set( vStart.AsVector3(), vEnd.AsVector3(), vFollow.AsVector3() );

	TVector4 vSide = vFollow - vClosest;
	vSide.w        = vFollow.w;
	vSide.Normalise();

	TINT     iFrame1      = 0;
	TINT     iFrame2      = 0;
	TBOOL    bIntersected = TFALSE;
	TVector3 vIntersection;
	TINT     iLast = iCount - 1;

	if ( iLast < 1 )
	{
		iFrame2 = 1;
	}
	else
	{
		for ( ; iFrame2 < iLast; )
		{
			bIntersected = TMath::LineIntersectsPlane3D( oPlane, a_pCam->m_pKeyFrames[ iFrame2 ].m_vPos.AsVector3(), a_pCam->m_pKeyFrames[ iFrame2 + 1 ].m_vPos.AsVector3(), vIntersection );

			if ( bIntersected )
			{
				TVector4 vPoint( vIntersection.x, vIntersection.y, vIntersection.z, 1.0f );
				TFLOAT   fPointDist = TMath::ClippedClosestPointOnLine( vPoint, vStart, vEnd );

				TVector4 vPointOffset;
				vPointOffset.Multiply( vDir, fPointDist );

				TVector4 vPointSide( vIntersection.x - ( vPointOffset.x + vStart.x ), vIntersection.y - ( vPointOffset.y + vStart.y ), vIntersection.z - ( vPointOffset.z + vStart.z ), 1.0f );
				vPointSide.Normalise();

				// Only the intersection on the side of the followed position
				if ( 0.0f < vPointSide.y * vSide.y + vPointSide.x * vSide.x + vPointSide.z * vSide.z )
				{
					iFrame1 = iFrame2;
					a_rvPos.Set( vIntersection.x, vIntersection.y, vIntersection.z, 1.0f );
					iFrame2 = iFrame2 + 1;
					goto INTERSECTED;
				}

				bIntersected = TFALSE;
			}

			iFrame2++;
		}

		iFrame2 = 1;
	}

INTERSECTED:
	if ( a_bLooped && !bIntersected )
	{
		// Between the last and the first key frame
		iFrame1      = 0;
		bIntersected = TMath::LineIntersectsPlane3D( oPlane, a_pCam->m_pKeyFrames[ 0 ].m_vPos.AsVector3(), a_pCam->m_pKeyFrames[ iLast ].m_vPos.AsVector3(), vIntersection );

		if ( bIntersected )
		{
			a_rvPos.Set( vIntersection.x, vIntersection.y, vIntersection.z, 1.0f );
			iFrame2 = iLast;
		}
	}

	if ( !bIntersected )
	{
		// The closest end
		TFLOAT fFirstDistSq = TVector4::DistanceSq( a_pCam->m_pKeyFrames[ 0 ].m_vPos, a_rvFollowPos );
		TFLOAT fLastDistSq  = TVector4::DistanceSq( a_pCam->m_pKeyFrames[ iCount - 1 ].m_vPos, a_rvFollowPos );

		if ( fLastDistSq <= fFirstDistSq )
			SetFromKeyFrame( a_pCam->m_pKeyFrames[ iCount - 1 ], a_rvPos, a_rqOrient, a_rfFOV );
		else
			SetFromKeyFrame( a_pCam->m_pKeyFrames[ 0 ], a_rvPos, a_rqOrient, a_rfFOV );

		return;
	}

	InterpolateKeyFrames( a_pCam, a_rvPos, a_rqOrient, a_rfFOV, iFrame1, iFrame2, a_bLooped );
}

// $deBlob: FUNCTION 00596210
void ASSCameraFunction::InterpolateAlongPath( const Camera* a_pCam, const TVector4& a_rvFollowPos, const TVector4& a_rvViewPos, TVector4& a_rvPos, TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TFLOAT a_fRadius, TFLOAT a_fMinOffset, TFLOAT a_fOffsetRadiantAngle )
{
	TASSERT( a_pCam->m_iCount > 0 );
	TASSERT( a_fOffsetRadiantAngle >= 0.0f && a_fOffsetRadiantAngle < TMath::HALF_PI );

	TINT            iCount     = a_pCam->m_iCount;
	const KeyFrame* pKeyFrames = a_pCam->m_pKeyFrames;

	if ( iCount != 1 )
	{
		const KeyFrame& rLast = pKeyFrames[ iCount - 1 ];

		// Direction from the first to the last key frame
		TVector4 vDir( rLast.m_vPos.x - pKeyFrames[ 0 ].m_vPos.x, rLast.m_vPos.y - pKeyFrames[ 0 ].m_vPos.y, rLast.m_vPos.z - pKeyFrames[ 0 ].m_vPos.z, rLast.m_vPos.w );
		TFLOAT   fLength = vDir.NormaliseMag();
		pKeyFrames       = a_pCam->m_pKeyFrames;

		if ( TMath::MINFLOAT < fLength )
		{
			const TVector4& rvFirst = pKeyFrames[ 0 ].m_vPos;
			TFLOAT          fDist   = vDir.y * ( a_rvFollowPos.y - rvFirst.y ) + vDir.x * ( a_rvFollowPos.x - rvFirst.x ) + vDir.z * ( a_rvFollowPos.z - rvFirst.z );

			if ( 0.0f < fDist )
			{
				if ( !( fDist < fLength ) )
				{
					// Past the last key frame
					SetFromKeyFrame( pKeyFrames[ iCount - 1 ], a_rvPos, a_rqOrient, a_rfFOV );
					return;
				}

				// The followed position projected on the direction
				TVector4 vOffset;
				vOffset.Multiply( vDir, fDist );
				TVector4 vProjected( vOffset.x + rvFirst.x, vOffset.y + rvFirst.y, vOffset.z + rvFirst.z, rvFirst.w );

				TFLOAT fOffset   = 0.0f;
				TFLOAT fDistance = TVector4::Distance( vProjected, a_rvFollowPos );

				if ( fDistance < a_fRadius )
					fOffset = TMath::Sqrt( a_fRadius * a_fRadius - fDistance * fDistance );

				if ( fOffset < a_fMinOffset )
					fOffset = a_fMinOffset;

				TFLOAT fAngleOffset = TMath::Tan( a_fOffsetRadiantAngle ) * fDistance;

				if ( fOffset < fAngleOffset )
					fOffset = fAngleOffset;

				TVector4 vPlanePoint = vProjected;

				if ( 0.0f < fOffset )
				{
					// The closer of the two points before and after it
					TFLOAT fBefore = fDist - fOffset;
					TFLOAT fAfter  = fOffset + fDist;
					TMath::Clip( fBefore, 0.0f, fLength );
					TMath::Clip( fAfter, 0.0f, fLength );

					pKeyFrames       = a_pCam->m_pKeyFrames;
					TVector4 vBefore = pKeyFrames[ 0 ].m_vPos;
					TVector4 vAfter  = pKeyFrames[ 0 ].m_vPos;

					vOffset.Multiply( vDir, fBefore );
					vBefore.x += vOffset.x;
					vBefore.y += vOffset.y;
					vBefore.z += vOffset.z;

					vOffset.Multiply( vDir, fAfter );
					vAfter.x += vOffset.x;
					vAfter.y += vOffset.y;
					vAfter.z += vOffset.z;

					TFLOAT fBeforeDistSq = TVector4::DistanceSq( vBefore, a_rvPos );
					TFLOAT fAfterDistSq  = TVector4::DistanceSq( vAfter, a_rvPos );

					vPlanePoint = ( fBeforeDistSq < fAfterDistSq ) ? vBefore : vAfter;
				}

				TPlane oPlane( vDir.AsVector3(), vPlanePoint.x * vDir.x + vPlanePoint.y * vDir.y + vPlanePoint.z * vDir.z );

				TBOOL    bHasIntersected = TFALSE;
				TINT     iFrame1         = 0;
				TINT     iFrame2         = 1;
				TVector3 vIntersection;

				for ( TINT i = 0; iCount != 1 && i < iCount - 1; i++ )
				{
					bHasIntersected = TMath::LineIntersectsPlane3D( oPlane, a_pCam->m_pKeyFrames[ i ].m_vPos.AsVector3(), a_pCam->m_pKeyFrames[ i + 1 ].m_vPos.AsVector3(), vIntersection );

					if ( bHasIntersected )
					{
						iFrame1 = i;
						iFrame2 = i + 1;
						a_rvPos.Set( vIntersection.x, vIntersection.y, vIntersection.z, 1.0f );
						break;
					}
				}

				TASSERT( bHasIntersected );

				if ( bHasIntersected )
				{
					InterpolateKeyFrames( a_pCam, a_rvPos, a_rqOrient, a_rfFOV, iFrame1, iFrame2, TFALSE );
					return;
				}

				// The closest end
				TFLOAT fFirstDistSq = TVector4::DistanceSq( a_pCam->m_pKeyFrames[ 0 ].m_vPos, a_rvFollowPos );
				TFLOAT fLastDistSq  = TVector4::DistanceSq( a_pCam->m_pKeyFrames[ iCount - 1 ].m_vPos, a_rvFollowPos );

				if ( fFirstDistSq < fLastDistSq )
					SetFromKeyFrame( a_pCam->m_pKeyFrames[ 0 ], a_rvPos, a_rqOrient, a_rfFOV );
				else
					SetFromKeyFrame( a_pCam->m_pKeyFrames[ iCount - 1 ], a_rvPos, a_rqOrient, a_rfFOV );

				return;
			}
		}
	}

	SetFromKeyFrame( pKeyFrames[ 0 ], a_rvPos, a_rqOrient, a_rfFOV );
}

// $deBlob: FUNCTION 00596180
void ASSCameraFunction::GetCameraTransform( const Camera* a_pCam, const TVector4& a_rvLineStart, const TVector4& a_rvLineEnd, const TVector4& a_rvFollowPos, const TVector4& a_rvViewPos, TVector4& a_rvPos, TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TFLOAT a_fRadius, TFLOAT a_fMinOffset, TFLOAT a_fOffsetRadiantAngle, TFLOAT a_fLineRadius, TUINT32 a_uiFlags )
{
	if ( a_uiFlags & IF_PATH )
	{
		InterpolateAlongPath( a_pCam, a_rvFollowPos, a_rvViewPos, a_rvPos, a_rqOrient, a_rfFOV, a_fRadius, a_fMinOffset, a_fOffsetRadiantAngle );
		return;
	}

	TBOOL bLooped;

	if ( a_uiFlags & IF_LINE )
		bLooped = TFALSE;
	else if ( a_uiFlags & IF_LINE_LOOPED )
		bLooped = TTRUE;
	else
		return;

	InterpolateAlongLine( a_pCam, a_rvLineStart, a_rvLineEnd, a_rvFollowPos, a_rvViewPos, a_rvPos, a_rqOrient, a_rfFOV, a_fLineRadius, bLooped );
}

// $deBlob: FUNCTION 00595180
TBOOL ASSCameraFunction::GetPathKeyFrame( const Camera* a_pCam, TFLOAT a_fTime, KeyFrame* a_pOut, const KeyFrame** a_ppCurrent, const KeyFrame** a_ppNext, const KeyFrame** a_ppNextNext )
{
	*a_ppCurrent  = TNULL;
	*a_ppNext     = TNULL;
	*a_ppNextNext = TNULL;

	TINT iCount = a_pCam->m_iCount;

	if ( iCount < 1 )
		return TFALSE;

	// The last key frame at or before a_fTime
	TINT iFrame = 0;

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( a_fTime < a_pCam->m_pKeyFrames[ i ].m_fTime )
			break;

		iFrame = i;
	}

	TINT iLast     = iCount - 1;
	TINT iPrev     = iFrame - 1;
	TINT iNext     = iFrame + 1;
	TINT iNextNext = iFrame + 2;
	TMath::Clip( iPrev, 0, iLast );
	TMath::Clip( iNext, 0, iLast );
	TMath::Clip( iNextNext, 0, iLast );

	const KeyFrame* pNext = &a_pCam->m_pKeyFrames[ iNext ];

	if ( pNext->m_iUsePrev == 0 )
		iPrev = iFrame;

	if ( pNext->m_iUseNext == 0 )
		iNextNext = iNext;

	const KeyFrame* pPrev     = &a_pCam->m_pKeyFrames[ iPrev ];
	const KeyFrame* pCurrent  = &a_pCam->m_pKeyFrames[ iFrame ];
	const KeyFrame* pNextNext = &a_pCam->m_pKeyFrames[ iNextNext ];
	*a_ppCurrent              = pCurrent;
	*a_ppNext                 = pNext;
	*a_ppNextNext             = pNextNext;

	if ( iFrame == iLast )
	{
		*a_pOut = *pCurrent;
		return TTRUE;
	}

	TFLOAT fGap = pNext->m_fTime - pCurrent->m_fTime;
	TASSERT( fGap >= 0.0f );

	if ( 0.0f < fGap )
	{
		TFLOAT fdiff = a_fTime - pCurrent->m_fTime;
		TASSERT( fdiff >= 0 && fdiff <= fGap );

		TFLOAT fPosT = EvaluateCurve( pNext->m_pPosCurve, pNext->m_iNumPosCurve, fdiff / fGap );
		HermiteKeyFrames( a_pOut->m_vPos, &pPrev->m_vPos, &pCurrent->m_vPos, &pNext->m_vPos, &pNextNext->m_vPos, pPrev->m_fTime, pCurrent->m_fTime, pNext->m_fTime, pNextNext->m_fTime, pPrev->m_fTension, pCurrent->m_fTension, pNext->m_fTension, pNextNext->m_fTension, fPosT );

		TFLOAT fOrientT = EvaluateCurve( pNext->m_pOrientCurve, pNext->m_iNumOrientCurve, fdiff / fGap );
		InterpolatePathOrientation( a_pCam, a_pOut->m_qOrient, pPrev, pCurrent, pNext, pNextNext, fOrientT );

		a_pOut->m_fFade  = ( pNext->m_fFade - pCurrent->m_fFade ) * fOrientT + pCurrent->m_fFade;
		a_pOut->m_fUnk28 = fdiff;
		a_pOut->m_fFOV   = ( pNext->m_fFOV - pCurrent->m_fFOV ) * fOrientT + pCurrent->m_fFOV;
		a_pOut->m_fTime  = a_fTime;
		return TTRUE;
	}

	*a_pOut = *pNext;
	return TTRUE;
}

// $deBlob: FUNCTION 005954e0
void ASSCameraFunction::InterpolatePathOrientation( const Camera* a_pCam, TQuaternion& a_rqOut, const KeyFrame* a_pFrame0, const KeyFrame* a_pFrame1, const KeyFrame* a_pFrame2, const KeyFrame* a_pFrame3, TFLOAT a_fT )
{
	if ( a_pFrame2->m_iUsePrev == 0 )
		a_pFrame0 = a_pFrame1;

	if ( a_pFrame2->m_iUseNext == 0 )
		a_pFrame3 = a_pFrame2;

	TMatrix44 mMat;
	mMat.SetFromQuaternion( a_pFrame0->m_qOrient );
	TVector4 vForward0 = mMat.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4 vUp0      = mMat.AsBasisVector4( BASISVECTOR_UP );
	mMat.SetFromQuaternion( a_pFrame1->m_qOrient );
	TVector4 vForward1 = mMat.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4 vUp1      = mMat.AsBasisVector4( BASISVECTOR_UP );
	mMat.SetFromQuaternion( a_pFrame2->m_qOrient );
	TVector4 vForward2 = mMat.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4 vUp2      = mMat.AsBasisVector4( BASISVECTOR_UP );
	mMat.SetFromQuaternion( a_pFrame3->m_qOrient );
	TVector4 vForward3 = mMat.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4 vUp3      = mMat.AsBasisVector4( BASISVECTOR_UP );

	TVector4 vForward;
	HermiteKeyFrames( vForward, &vForward0, &vForward1, &vForward2, &vForward3, a_pFrame0->m_fTime, a_pFrame1->m_fTime, a_pFrame2->m_fTime, a_pFrame3->m_fTime, a_pFrame0->m_fTension, a_pFrame1->m_fTension, a_pFrame2->m_fTension, a_pFrame3->m_fTension, a_fT );
	vForward.Normalise();

	TVector4 vUp;
	HermiteKeyFrames( vUp, &vUp0, &vUp1, &vUp2, &vUp3, a_pFrame0->m_fTime, a_pFrame1->m_fTime, a_pFrame2->m_fTime, a_pFrame3->m_fTime, a_pFrame0->m_fTension, a_pFrame1->m_fTension, a_pFrame2->m_fTension, a_pFrame3->m_fTension, a_fT );
	vUp.Normalise();

	TVector4 vRight;
	vRight.CrossProduct( vUp, vForward );
	vRight.Normalise();
	vUp.CrossProduct( vForward, vRight );
	vUp.Normalise();

	TMatrix44 mOrient;
	mOrient.AsBasisVector4( BASISVECTOR_RIGHT )       = TVector4( vRight.x, vRight.y, vRight.z, 0.0f );
	mOrient.AsBasisVector4( BASISVECTOR_UP )          = TVector4( vUp.x, vUp.y, vUp.z, 0.0f );
	mOrient.AsBasisVector4( BASISVECTOR_FORWARD )     = TVector4( vForward.x, vForward.y, vForward.z, 0.0f );
	mOrient.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TMatrix44::IDENTITY.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TMatrix44::MatToQuat( a_rqOut, mOrient );
}

// $deBlob: FUNCTION 00595790
TBOOL ASSCameraFunction::GetKeyFrame( const Camera* a_pCam, TFLOAT a_fTime, KeyFrame* a_pOut, const KeyFrame** a_ppCurrent, const KeyFrame** a_ppPrev, const KeyFrame** a_ppNext, const KeyFrame** a_ppNextNext )
{
	TINT iCount   = a_pCam->m_iCount;
	*a_ppCurrent  = TNULL;
	*a_ppPrev     = TNULL;
	*a_ppNext     = TNULL;
	*a_ppNextNext = TNULL;

	if ( iCount < 1 )
		return TFALSE;

	TINT iLast = iCount - 1;

	// The last key frame at or before a_fTime
	TINT iFrame = 0;

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( a_fTime < a_pCam->m_pKeyFrames[ i ].m_fTime )
			break;

		iFrame = i;
	}

	TINT iPrev     = iFrame - 1;
	TINT iNext     = iFrame + 1;
	TINT iNextNext = iFrame + 2;
	TMath::Clip( iPrev, 0, iLast );
	TMath::Clip( iNext, 0, iLast );
	TMath::Clip( iNextNext, 0, iLast );

	const KeyFrame* pCurrent  = &a_pCam->m_pKeyFrames[ iFrame ];
	const KeyFrame* pNext     = &a_pCam->m_pKeyFrames[ iNext ];
	const KeyFrame* pNextNext = &a_pCam->m_pKeyFrames[ iNextNext ];
	const KeyFrame* pPrev     = &a_pCam->m_pKeyFrames[ iPrev ];

	if ( iFrame == iLast )
	{
		*a_pOut       = *pCurrent;
		*a_ppCurrent  = pCurrent;
		*a_ppPrev     = pPrev;
		*a_ppNext     = pNext;
		*a_ppNextNext = pNextNext;
		return TTRUE;
	}

	TFLOAT fGap = pNext->m_fTime - pCurrent->m_fTime;
	TASSERT( fGap >= 0.0f );

	if ( fGap <= 0.0f )
	{
		*a_pOut       = *pNext;
		*a_ppCurrent  = pNext;
		*a_ppPrev     = pCurrent;
		*a_ppNext     = pNextNext;
		*a_ppNextNext = pNextNext;
		return TTRUE;
	}

	TFLOAT fdiff = a_fTime - pCurrent->m_fTime;
	TASSERT( fdiff >= 0 && fdiff <= fGap );

	TFLOAT fPercent = fdiff / fGap;
	TASSERT( fPercent >= 0.0f && fPercent <= 1.0f );
	TMath::Clip( fPercent, 0.0f, 1.0f );

	if ( 0.001f < TVector4::Distance( pCurrent->m_vPos, pNext->m_vPos ) )
		CatmullRom( a_pOut->m_vPos, pPrev->m_vPos, pCurrent->m_vPos, pNext->m_vPos, pNextNext->m_vPos, fPercent );
	else
		a_pOut->m_vPos = pNext->m_vPos;

	TQuaternion::Slerp( a_pOut->m_qOrient, pCurrent->m_qOrient, pNext->m_qOrient, fPercent );
	a_pOut->m_fFOV  = ( pNext->m_fFOV - pCurrent->m_fFOV ) * fPercent + pCurrent->m_fFOV;
	a_pOut->m_fFade = ( pNext->m_fFade - pCurrent->m_fFade ) * fPercent + pCurrent->m_fFade;
	a_pOut->m_fTime = a_fTime;

	*a_ppCurrent  = pCurrent;
	*a_ppPrev     = pPrev;
	*a_ppNext     = pNext;
	*a_ppNextNext = pNextNext;
	return TTRUE;
}

// $deBlob: FUNCTION 00595b00
ASSCameraFunction::KeyFrame* ASSCameraFunction::GetKeyFrame( const Camera* a_pCam, TINT a_iKeyFrame )
{
	TASSERT( a_iKeyFrame < a_pCam->m_iCount && a_iKeyFrame >= 0 );

	if ( a_pCam != TNULL && a_iKeyFrame < a_pCam->m_iCount && a_iKeyFrame >= 0 )
		return &a_pCam->m_pKeyFrames[ a_iKeyFrame ];

	return TNULL;
}

// $deBlob: FUNCTION 00595b70
TFLOAT ASSCameraFunction::GetTotalTime( const Camera* a_pCam )
{
	if ( a_pCam != TNULL && 0 < a_pCam->m_iCount )
		return a_pCam->m_pKeyFrames[ a_pCam->m_iCount - 1 ].m_fTime;

	return 0.0f;
}

// $deBlob: FUNCTION 00595ba0
TINT ASSCameraFunction::GetNumKeyFrames( const Camera* a_pCam )
{
	if ( a_pCam != TNULL )
		return a_pCam->m_iCount;

	return 0;
}

// The number of cameras of the level, 0 if none are loaded
// $deBlob: FUNCTION 00594ea0
TINT ASSCameraFunction::GetNumCameras() const
{
	return ( m_pHeader != TNULL ) ? m_pHeader->m_iNumCameras : 0;
}

// $deBlob: FUNCTION 00594eb0
TFLOAT ASSCameraFunction::EvaluateCurve( const TFLOAT* a_pCurve, TINT a_iCount, TFLOAT a_fTime )
{
	TASSERT( a_iCount >= 4 );

	if ( a_iCount < 4 )
		return a_fTime;

	// The first point (from the third one) at or after a_fTime, the last inner one if none
	TINT iLast = a_iCount - 1;
	TINT iNext = 2;

	for ( TINT i = 2; i < iLast; i++ )
	{
		iNext = i;

		if ( a_fTime <= a_pCurve[ i * 2 ] )
			break;
	}

	TINT iPrev = iNext - 1;
	TASSERT( iPrev > 0 );
	TASSERT( iNext < ( a_iCount - 1 ) );

	TFLOAT fGap    = a_pCurve[ iNext * 2 ] - a_pCurve[ iPrev * 2 ];
	TFLOAT fdiff   = a_fTime - a_pCurve[ iPrev * 2 ];
	TFLOAT fResult = a_pCurve[ iNext * 2 + 1 ];

	if ( 0.0f < fGap )
	{
		if ( fdiff <= 0.0f )
		{
			fResult = a_pCurve[ iPrev * 2 + 1 ];
		}
		else if ( 0.0f < fGap - fdiff )
		{
			// The segment's direction: the time is scaled by its length along the time axis
			TVector2 vDir( a_pCurve[ iNext * 2 ] - a_pCurve[ iPrev * 2 ], a_pCurve[ iNext * 2 + 1 ] - a_pCurve[ iPrev * 2 + 1 ] );
			TFLOAT   fLength = vDir.NormalizeMag();

			if ( 0.0f < fLength )
			{
				TFLOAT fDot = vDir.y * 0.0f + vDir.x;

				if ( fDot <= 0.0f )
				{
					fResult = a_pCurve[ iPrev * 2 + 1 ];
				}
				else
				{
					// Catmull-Rom of the values
					TFLOAT fT  = ( fdiff / fDot ) / fLength;
					TFLOAT fV0 = a_pCurve[ iNext * 2 - 3 ];
					TFLOAT fV1 = a_pCurve[ iPrev * 2 + 1 ];
					TFLOAT fV2 = a_pCurve[ iNext * 2 + 1 ];
					TFLOAT fV3 = a_pCurve[ iNext * 2 + 3 ];

					fResult = ( ( fV2 - fV0 ) * fT + fV1 * 2.0f + ( ( ( fV0 * 2.0f - fV1 * 5.0f ) + fV2 * 4.0f ) - fV3 ) * fT * fT + ( ( fV3 + ( fV1 * 3.0f - fV2 * 3.0f ) ) - fV0 ) * fT * fT * fT ) * 0.5f;
				}
			}
		}
	}

	if ( 1.0f < fResult )
		return 1.0f;

	if ( fResult < 0.0f )
		fResult = 0.0f;

	return fResult;
}

// $deBlob: FUNCTION 005974d0
void ASSCameraFunction::HermiteKeyFrames( TVector4& a_rvOut, const TVector4* a_pvP0, const TVector4* a_pvP1, const TVector4* a_pvP2, const TVector4* a_pvP3, TFLOAT a_fTime0, TFLOAT a_fTime1, TFLOAT a_fTime2, TFLOAT a_fTime3, TFLOAT a_fTension0, TFLOAT a_fTension1, TFLOAT a_fTension2, TFLOAT a_fTension3, TFLOAT a_fT )
{
	// The time spans of the segments (one that isn't positive takes a neighbour's, else 1)
	TFLOAT fGap01 = a_fTime1 - a_fTime0;
	TFLOAT fGap12 = a_fTime2 - a_fTime1;
	TFLOAT fGap23 = a_fTime3 - a_fTime2;

	TFLOAT fInA = fGap12;
	TFLOAT fInB = fGap01;

	if ( 0.0f < fGap12 )
	{
		if ( fGap01 <= 0.0f )
			fInB = fInA;
	}
	else
	{
		fInA = fGap01;

		if ( fGap01 <= 0.0f )
		{
			fInA = 1.0f;
			fInB = fInA;
		}
	}

	TFLOAT fOutC = fGap23;
	TFLOAT fOutD = fGap12;

	if ( fGap23 <= 0.0f )
		fOutC = ( 0.0f < fGap12 ) ? fGap12 : 1.0f;

	if ( fGap12 <= 0.0f )
		fOutD = fOutC;

	TFLOAT fInvInB  = 1.0f / fInB;
	TFLOAT fInvInA  = 1.0f / fInA;
	TFLOAT fInvOutD = 1.0f / fOutD;
	TFLOAT fInvOutC = 1.0f / fOutC;

	a_rvOut = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	// Each component is a 2D curve (value, time)
	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fP0 = ( &a_pvP0->x )[ i ];
		TFLOAT fP1 = ( &a_pvP1->x )[ i ];
		TFLOAT fP2 = ( &a_pvP2->x )[ i ];
		TFLOAT fP3 = ( &a_pvP3->x )[ i ];

		TVector4 vPoint1( fP1, a_fTime1, 0.0f, 1.0f );
		TVector4 vPoint2( fP2, a_fTime2, 0.0f, 1.0f );

		// NOTE: retail scales the incoming half of each tangent by both spans and the outgoing half by none
		TVector4 vTangent1( fP1 - fP0, fGap01, 0.0f, 0.0f );
		vTangent1.Multiply( 0.5f );
		vTangent1.Multiply( fInvInB );
		TVector4 vHalf( fP2 - fP1, fGap12, 0.0f, 0.0f );
		vHalf.Multiply( 0.5f );
		vTangent1.Multiply( fInvInA );
		vTangent1.x = vHalf.x + vTangent1.x;
		vTangent1.y = vHalf.y + vTangent1.y;
		vTangent1.z = vHalf.z + vTangent1.z;
		vTangent1.Multiply( a_fTension1 );

		TVector4 vTangent2( fP2 - fP1, fGap12, 0.0f, 0.0f );
		vTangent2.Multiply( 0.5f );
		vTangent2.Multiply( fInvOutD );
		vHalf = TVector4( fP3 - fP2, fGap23, 0.0f, 0.0f );
		vHalf.Multiply( 0.5f );
		vTangent2.Multiply( fInvOutC );
		vTangent2.x = vHalf.x + vTangent2.x;
		vTangent2.y = vHalf.y + vTangent2.y;
		vTangent2.z = vHalf.z + vTangent2.z;
		vTangent2.Multiply( a_fTension2 );

		TVector4 vResult;
		Hermite( vResult, vPoint1, vPoint2, vTangent1, vTangent2, a_fT );
		( &a_rvOut.x )[ i ] = vResult.x;
	}
}

// $deBlob: FUNCTION 00597170
void ASSCameraFunction::Hermite( TVector4& a_rvOut, TVector4 a_vP1, TVector4 a_vP2, TVector4 a_vT1, TVector4 a_vT2, TFLOAT a_fT )
{
	TFLOAT fT2 = a_fT * a_fT;
	TFLOAT fT3 = fT2 * a_fT;

	a_vP1.Multiply( ( fT3 * 2.0f - fT2 * 3.0f ) + 1.0f );
	a_vP2.Multiply( fT3 * -2.0f + fT2 * 3.0f );
	a_vT1.Multiply( ( fT3 - fT2 * 2.0f ) + a_fT );
	a_vT2.Multiply( fT3 - fT2 );

	a_rvOut   = a_vP1;
	a_rvOut.x = a_rvOut.x + a_vP2.x + a_vT1.x + a_vT2.x;
	a_rvOut.y = a_vP2.y + a_rvOut.y + a_vT1.y + a_vT2.y;
	a_rvOut.z = a_vP2.z + a_rvOut.z + a_vT1.z + a_vT2.z;
}

// $deBlob: FUNCTION 00597290
// $deBlob: FUNCTION 005973b0
void ASSCameraFunction::CalcTangent( TVector4& a_rvOut, const TVector4& a_rvP0, const TVector4& a_rvP1, const TVector4& a_rvP2, TFLOAT a_fTime0, TFLOAT a_fTime1, TFLOAT a_fScale )
{
	a_rvOut = TVector4( a_rvP1.x - a_rvP0.x, a_rvP1.y - a_rvP0.y, a_rvP1.z - a_rvP0.z, a_rvP1.w - a_rvP0.w );
	a_rvOut.Multiply( 0.5f );
	a_rvOut.Multiply( 1.0f / a_fTime0 );

	TVector4 vNext( a_rvP2.x - a_rvP1.x, a_rvP2.y - a_rvP1.y, a_rvP2.z - a_rvP1.z, a_rvP2.w - a_rvP1.w );
	vNext.Multiply( 0.5f );

	// NOTE: retail scales a_rvOut instead of vNext here (dead code, kept as is)
	a_rvOut.Multiply( 1.0f / a_fTime1 );

	a_rvOut.x += vNext.x;
	a_rvOut.y += vNext.y;
	a_rvOut.z += vNext.z;
	a_rvOut.Multiply( a_fScale );
}

// $deBlob: FUNCTION 00594c80
void ASSCameraFunction::CatmullRom( TVector2& a_rvOut, const TVector2& a_rvP0, const TVector2& a_rvP1, const TVector2& a_rvP2, const TVector2& a_rvP3, TFLOAT a_fT )
{
	TFLOAT fT2 = a_fT * a_fT;

	a_rvOut.x = ( ( a_rvP2.x - a_rvP0.x ) * a_fT + a_rvP1.x * 2.0f + ( ( ( a_rvP0.x * 2.0f - a_rvP1.x * 5.0f ) + a_rvP2.x * 4.0f ) - a_rvP3.x ) * fT2 + ( ( a_rvP3.x + ( a_rvP1.x * 3.0f - a_rvP2.x * 3.0f ) ) - a_rvP0.x ) * fT2 * a_fT ) * 0.5f;
	a_rvOut.y = ( ( a_rvP2.y - a_rvP0.y ) * a_fT + a_rvP1.y * 2.0f + ( ( ( a_rvP0.y * 2.0f - a_rvP1.y * 5.0f ) + a_rvP2.y * 4.0f ) - a_rvP3.y ) * fT2 + ( ( a_rvP3.y + ( a_rvP1.y * 3.0f - a_rvP2.y * 3.0f ) ) - a_rvP0.y ) * fT2 * a_fT ) * 0.5f;
}
