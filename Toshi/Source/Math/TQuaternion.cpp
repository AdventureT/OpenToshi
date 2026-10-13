#include "ToshiPCH.h"
#include "TQuaternion.h"
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

const TQuaternion TQuaternion::IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

// $deBlob: FUNCTION 00675f40
void TQuaternion::SetFromEulerYX( const TFLOAT* fVal )
{
	TFLOAT f1;
	TFLOAT f2;

	TMath::SinCos( fVal[ 0 ] * 0.5f, f1, f2 );
	TQuaternion quat = TQuaternion( f1, 0, 0, f2 );
	TMath::SinCos( fVal[ 1 ] * 0.5f, f1, f2 );
	Set( 0, f1, 0, f2 );
	*this *= quat;
}

// $deBlob: FUNCTION 00676090
// $Barnyard: FUNCTION 006c9a30
void TQuaternion::SetFromEulerRollPitchYaw( TFLOAT a_fRoll, TFLOAT a_fPitch, TFLOAT a_fYaw )
{
	TFLOAT fSinRoll, fCosRoll;
	TFLOAT fSinPitch, fCosPitch;
	TFLOAT fSinYaw, fCosYaw;
	TMath::SinCos( a_fRoll * 0.5f, fSinRoll, fCosRoll );
	TMath::SinCos( a_fPitch * 0.5f, fSinPitch, fCosPitch );
	TMath::SinCos( a_fYaw * 0.5f, fSinYaw, fCosYaw );

	TFLOAT fX = fCosYaw * fCosPitch * fSinRoll - fSinYaw * fSinPitch * fCosRoll;
	TFLOAT fW = fSinYaw * fSinPitch * fSinRoll + fCosYaw * fCosPitch * fCosRoll;
	TFLOAT fY = fCosPitch * fSinRoll * fSinYaw + fSinPitch * fCosRoll * fCosYaw;
	TFLOAT fZ = fCosPitch * fCosRoll * fSinYaw - fSinPitch * fSinRoll * fCosYaw;

	Set( fX, fY, fZ, fW );
}

// $deBlob: FUNCTION 00674c50
// $Barnyard: FUNCTION 006c93b0
void TQuaternion::SetRotation( const TVector3& a_rVec3, TFLOAT a_fVal )
{
	TFLOAT fVal;
	TFLOAT fVal2;

	TMath::SinCos( a_fVal * 0.5f, fVal, fVal2 );
	Set( a_rVec3.x * fVal, a_rVec3.y * fVal, a_rVec3.z * fVal, fVal2 );
}

// $deBlob: FUNCTION 00675880
// $Barnyard: FUNCTION 006c9840
void TQuaternion::SetVectorDirection( const TVector3& a_rvDest, const TVector3& a_rvSource )
{
	TASSERT( ( a_rvDest.Magnitude() > 0.95f ) && ( a_rvDest.Magnitude() < 1.05f ) );
	TASSERT( ( a_rvSource.Magnitude() > 0.95f ) && ( a_rvSource.Magnitude() < 1.05f ) );

	TVector3 vAxis;
	vAxis.x = a_rvSource.y * a_rvDest.z - a_rvSource.z * a_rvDest.y;
	vAxis.y = a_rvDest.x * a_rvSource.z - a_rvSource.x * a_rvDest.z;
	vAxis.z = a_rvSource.x * a_rvDest.y - a_rvDest.x * a_rvSource.y;

	TFLOAT fMag = vAxis.Magnitude();

	if ( fMag > TMath::TFLOAT_EPSILON )
	{
		TFLOAT fOneOverMag = 1.0f / fMag;
		TFLOAT fDot        = a_rvDest.x * a_rvSource.x + a_rvSource.y * a_rvDest.y + a_rvSource.z * a_rvDest.z;
		TFLOAT fAngle;

		if ( fDot <= 1.0f - TMath::TFLOAT_EPSILON )
			fAngle = ( -1.0f + TMath::TFLOAT_EPSILON <= fDot ) ? TMath::ACos( fDot ) : TMath::PI;
		else
			fAngle = 0.0f;

		TFLOAT fSin, fCos;
		TMath::SinCos( fAngle * 0.5f, fSin, fCos );
		x = fSin * fOneOverMag * vAxis.x;
		y = vAxis.y * fOneOverMag * fSin;
		z = fSin * fOneOverMag * vAxis.z;
		w = fCos;
		return;
	}

	if ( 0.0f <= a_rvDest.x * a_rvSource.x + a_rvDest.y * a_rvSource.y + a_rvSource.z * a_rvDest.z )
	{
		// Same direction (or a zero vector): no rotation
		Set( 0.0f, 0.0f, 0.0f, 1.0f );
		return;
	}

	TVector3 vec = a_rvSource;
	if ( TMath::Abs( a_rvSource.y ) < 0.95f )
		vec.RotateY( TMath::HALF_PI );
	else
		vec.RotateX( TMath::HALF_PI );

	SetIdentity();
	RotateAroundAxis( vec, TMath::PI );
}

// $deBlob: FUNCTION 00675470
// $Barnyard: FUNCTION 006c9650
void TQuaternion::RotateVector( TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3 )
{
	// NOTE: de Blob writes q * v * conj( q ) out with this order of operations
	TFLOAT fX = param_2.x, fY = param_2.y, fZ = param_2.z, fW = param_2.w;
	TFLOAT fNX = -fX, fNY = -fY, fNZ = -fZ;

	TFLOAT fTX = ( fW * param_3.x + fX * 0.0f + fY * param_3.z ) - fZ * param_3.y;
	TFLOAT fTY = ( fW * param_3.y + fY * 0.0f + fZ * param_3.x ) - fX * param_3.z;
	TFLOAT fTW = ( ( fW * 0.0f - fX * param_3.x ) - fY * param_3.y ) - fZ * param_3.z;
	TFLOAT fTZ = ( fW * param_3.z + fZ * 0.0f + fX * param_3.y ) - fY * param_3.x;

	param_1.x = ( fW * fTX + fNX * fTW + fNZ * fTY ) - fNY * fTZ;
	param_1.y = ( fW * fTY + fNY * fTW + fNX * fTZ ) - fNZ * fTX;
	param_1.z = ( fW * fTZ + fNZ * fTW + fNY * fTX ) - fNX * fTY;
}

// $deBlob: FUNCTION 00675670
void TQuaternion::RotateVector( TVector4& a_rOut, const TQuaternion& a_rcQuat, const TVector4& a_rcVec )
{
	TFLOAT fX = a_rcQuat.x, fY = a_rcQuat.y, fZ = a_rcQuat.z, fW = a_rcQuat.w;
	TFLOAT fNX = -fX, fNY = -fY, fNZ = -fZ;

	TFLOAT fTX = ( fW * a_rcVec.x + fX * 0.0f + fY * a_rcVec.z ) - fZ * a_rcVec.y;
	TFLOAT fTY = ( fW * a_rcVec.y + fY * 0.0f + fZ * a_rcVec.x ) - fX * a_rcVec.z;
	TFLOAT fTW = ( ( fW * 0.0f - fX * a_rcVec.x ) - fY * a_rcVec.y ) - fZ * a_rcVec.z;
	TFLOAT fTZ = ( fW * a_rcVec.z + fZ * 0.0f + fX * a_rcVec.y ) - fY * a_rcVec.x;

	a_rOut.w = 1.0f;
	a_rOut.x = ( fW * fTX + fNX * fTW + fNZ * fTY ) - fNY * fTZ;
	a_rOut.y = ( fW * fTY + fNY * fTW + fNX * fTZ ) - fNZ * fTX;
	a_rOut.z = ( fW * fTZ + fNZ * fTW + fNY * fTX ) - fNX * fTY;
}

// $deBlob: FUNCTION 006749d0
// $deBlob: FUNCTION 00674b10
// $Barnyard: FUNCTION 006c9340
void TQuaternion::RotateAroundAxis( const TVector3& param_1, TFLOAT param_2 )
{
	TFLOAT fVal;
	TFLOAT fVal2;

	TMath::SinCos( param_2 * 0.5f, fVal, fVal2 );
	*this *= TQuaternion( param_1.x * fVal, param_1.y * fVal, param_1.z * fVal, fVal2 );
}

// $deBlob: FUNCTION 00675c80
void TQuaternion::GetEulerXYZ( TVector3& outVec ) const
{
	TMatrix44 matrix;
	matrix.SetFromQuaternion( *this );

	matrix.GetEulerXYZ( outVec );
}

// $deBlob: FUNCTION 006743e0
// $Barnyard: FUNCTION 006c9190
TQuaternion& TQuaternion::operator*=( const TQuaternion& a_Quat )
{
	// NOTE: de Blob's order of operations (from its inlined copies and operator* 00674dd0)
	TFLOAT fX = ( w * a_Quat.x + x * a_Quat.w + y * a_Quat.z ) - z * a_Quat.y;
	TFLOAT fY = ( w * a_Quat.y + y * a_Quat.w + z * a_Quat.x ) - x * a_Quat.z;
	TFLOAT fZ = ( w * a_Quat.z + z * a_Quat.w + x * a_Quat.y ) - y * a_Quat.x;
	TFLOAT fW = ( ( w * a_Quat.w - x * a_Quat.x ) - y * a_Quat.y ) - z * a_Quat.z;
	Set( fX, fY, fZ, fW );
	return *this;
}

// $deBlob: FUNCTION 00674dd0
void TQuaternion::Multiply( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 )
{
	a_rOut = a_rcQuat1;
	a_rOut *= a_rcQuat2;
}

// $deBlob: FUNCTION 00674d10
void TQuaternion::Add( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 )
{
	a_rOut   = a_rcQuat1;
	a_rOut.x = a_rOut.x + a_rcQuat2.x;
	a_rOut.y = a_rcQuat2.y + a_rOut.y;
	a_rOut.z = a_rcQuat2.z + a_rOut.z;
	a_rOut.w = a_rcQuat2.w + a_rOut.w;
}

// $deBlob: FUNCTION 00674d70
void TQuaternion::Subtract( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 )
{
	a_rOut = a_rcQuat1;
	a_rOut -= a_rcQuat2;
}

// $deBlob: FUNCTION 00674ef0
void TQuaternion::Divide( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 )
{
	a_rOut = a_rcQuat1;
	a_rOut /= a_rcQuat2;
}

// $deBlob: FUNCTION 00675100
// $Barnyard: FUNCTION 006c9ae0
void TQuaternion::Nlerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress )
{
	TFLOAT fDot = TVector4::DotProduct4( a_rStart.AsVector4(), a_rEnd.AsVector4() );

	if ( 0.0f <= fDot )
	{
		TVector4 vec = a_rEnd.AsVector4();

		vec.Substract4( a_rStart.AsVector4() );
		vec.Multiply4( vec, a_fProgress );

		a_rOut.x = a_rStart.x + vec.x;
		a_rOut.y = vec.y + a_rStart.y;
		a_rOut.z = a_rStart.z + vec.z;
		a_rOut.w = a_rStart.w + vec.w;
	}
	else
	{
		TVector4 vec = a_rEnd.AsVector4();
		vec.Add4( a_rStart.AsVector4() );
		vec.Multiply4( vec, a_fProgress );

		a_rOut.x = vec.x - a_rStart.x;
		a_rOut.y = vec.y - a_rStart.y;
		a_rOut.z = vec.z - a_rStart.z;
		a_rOut.w = vec.w - a_rStart.w;
	}

	// NOTE: de Blob normalises inline (x, y, z, w summed in this order)
	TFLOAT fX = a_rOut.x, fY = a_rOut.y, fZ = a_rOut.z, fW = a_rOut.w;
	TFLOAT fMag = TMath::Sqrt( fX * fX + fY * fY + fZ * fZ + fW * fW );

	if ( fMag != 0.0f )
	{
		TFLOAT fOneOverMag = 1.0f / fMag;
		a_rOut.x           = fX * fOneOverMag;
		a_rOut.y           = fY * fOneOverMag;
		a_rOut.z           = fZ * fOneOverMag;
		a_rOut.w           = fW * fOneOverMag;
	}
}

// $Barnyard: FUNCTION 006c9590
void TQuaternion::Lerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress )
{
	TQuaternion quatEnd = a_rEnd;

	if ( TVector4::DotProduct4( a_rStart.AsVector4(), a_rEnd.AsVector4() ) < 0.0f )
		quatEnd.Set( -a_rEnd.x, -a_rEnd.y, -a_rEnd.z, -a_rEnd.w );

	TFLOAT fStart = 1.0f - a_fProgress;
	a_rOut.x      = fStart * a_rStart.x + quatEnd.x * a_fProgress;
	a_rOut.y      = fStart * a_rStart.y + quatEnd.y * a_fProgress;
	a_rOut.z      = fStart * a_rStart.z + quatEnd.z * a_fProgress;
	a_rOut.w      = fStart * a_rStart.w + quatEnd.w * a_fProgress;
}

// $deBlob: FUNCTION 00675320
void TQuaternion::Lerp( const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress )
{
	TVector4 vec;

	if ( TVector4::DotProduct4( a_rStart.AsVector4(), a_rEnd.AsVector4() ) < 0.0f )
	{
		vec.Set( a_rStart.x + a_rEnd.x, a_rEnd.y + a_rStart.y, a_rEnd.z + a_rStart.z, a_rEnd.w + a_rStart.w );
		vec.Multiply4( vec, a_fProgress );

		x = vec.x - a_rStart.x;
		y = vec.y - a_rStart.y;
		z = vec.z - a_rStart.z;
		w = vec.w - a_rStart.w;
		return;
	}

	vec.Set( a_rEnd.x - a_rStart.x, a_rEnd.y - a_rStart.y, a_rEnd.z - a_rStart.z, a_rEnd.w - a_rStart.w );
	vec.Multiply4( vec, a_fProgress );

	x = a_rStart.x + vec.x;
	y = vec.y + a_rStart.y;
	z = a_rStart.z + vec.z;
	w = a_rStart.w + vec.w;
}

// $deBlob: FUNCTION 00674f50
// $Barnyard: FUNCTION 006c9400
void TQuaternion::Slerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress )
{
	if ( a_fProgress == 0.0f )
	{
		a_rOut = a_rStart;
		return;
	}

	if ( a_fProgress == 1.0f )
	{
		a_rOut = a_rEnd;
		return;
	}

	TFLOAT fDot         = TVector4::DotProduct4( a_rStart.AsVector4(), a_rEnd.AsVector4() );
	TBOOL  bNegativeDot = fDot < 0.0f;

	if ( bNegativeDot )
		fDot *= -1;

	TFLOAT fProg1 = a_fProgress;
	TFLOAT fProg2;

	if ( 1.0f - fDot <= 0.001f )
	{
		fProg2 = 1.0f - a_fProgress;
	}
	else
	{
		TFLOAT fAcos = TMath::ACos( fDot );
		TFLOAT fSin  = TMath::Sin( fAcos );

		fProg2 = TMath::Sin( ( 1.0f - a_fProgress ) * fAcos ) * ( 1.0f / fSin );
		fProg1 = TMath::Sin( fAcos * a_fProgress ) * ( 1.0f / fSin );
	}

	if ( bNegativeDot )
	{
		a_rOut.x = a_rStart.x * fProg2 - a_rEnd.x * fProg1;
		a_rOut.y = a_rStart.y * fProg2 - a_rEnd.y * fProg1;
		a_rOut.z = a_rStart.z * fProg2 - a_rEnd.z * fProg1;
		a_rOut.w = a_rStart.w * fProg2 - a_rEnd.w * fProg1;
	}
	else
	{
		a_rOut.x = a_rStart.x * fProg2 + a_rEnd.x * fProg1;
		a_rOut.y = a_rStart.y * fProg2 + a_rEnd.y * fProg1;
		a_rOut.z = a_rStart.z * fProg2 + a_rEnd.z * fProg1;
		a_rOut.w = a_rStart.w * fProg2 + a_rEnd.w * fProg1;
	}
}

TOSHI_NAMESPACE_END
