#include "pch.h"
#include "ANEDKeyframeInterpolation.h"

#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007e5600
void ANEDKeyframeInterpolation::InterpolateMatrix( const TransformKeyData* a_pKey1, const TransformKeyData* a_pKey2, TFLOAT a_time, TMatrix44& a_rOutMatrix )
{
	TASSERT( a_time >= 0 && a_time <= 1.0f );

	if ( a_pKey2 != TNULL && a_time > 0.0f )
	{
		TVector4 vTranslation;
		vTranslation.x = GetInterpolatedScalar( a_pKey1, a_pKey2, a_time, DataType_Transform, KeyComponent_TranslationX );
		vTranslation.y = GetInterpolatedScalar( a_pKey1, a_pKey2, a_time, DataType_Transform, KeyComponent_TranslationY );
		vTranslation.z = GetInterpolatedScalar( a_pKey1, a_pKey2, a_time, DataType_Transform, KeyComponent_TranslationZ );

		TVector4 vScale;
		vScale.x = GetInterpolatedScalar( a_pKey1, a_pKey2, a_time, DataType_Transform, KeyComponent_ScaleX );
		vScale.y = GetInterpolatedScalar( a_pKey1, a_pKey2, a_time, DataType_Transform, KeyComponent_ScaleY );
		vScale.z = GetInterpolatedScalar( a_pKey1, a_pKey2, a_time, DataType_Transform, KeyComponent_ScaleZ );
		vTranslation.w = 1.0f;
		vScale.w       = 1.0f;

		TQuaternion quatRotation1;
		TMatrix44::MatToQuat( quatRotation1, *a_pKey1->pMatrix );

		TQuaternion quatRotation2;
		TMatrix44::MatToQuat( quatRotation2, *a_pKey2->pMatrix );

		TQuaternion quatRotation;
		TQuaternion::Slerp( quatRotation, quatRotation1, quatRotation2, a_time );

		a_rOutMatrix.SetFromQuaternion( quatRotation );
		a_rOutMatrix.Scale( vScale );
		a_rOutMatrix.m_f41 = vTranslation.x;
		a_rOutMatrix.m_f42 = vTranslation.y;
		a_rOutMatrix.m_f43 = vTranslation.z;
		a_rOutMatrix.m_f44 = vTranslation.w;
		return;
	}

	a_rOutMatrix = *a_pKey1->pMatrix;
}

// $deBlob: FUNCTION 007e57b0
TFLOAT ANEDKeyframeInterpolation::GetInterpolatedScalar( const TransformKeyData* a_source, const TransformKeyData* a_pKey2, TFLOAT a_time, DataType a_eDataType, TINT a_component )
{
	TASSERT( a_time >= 0 && a_time <= 1.0f );
	TASSERT( a_component >= 0 && a_component < a_source->componentCount );

	TINT16 eInterpolation = a_source->pInterpolationTypes[ a_component ];

	if ( eInterpolation == Interpolation_Linear )
	{
		TFLOAT fValue1 = GetRootScalar( a_source, a_eDataType, a_component );
		TFLOAT fValue2 = GetRootScalar( a_pKey2, a_eDataType, a_component );
		return ( fValue2 - fValue1 ) * a_time + fValue1;
	}

	if ( eInterpolation != Interpolation_None && eInterpolation == Interpolation_Step )
		return GetRootScalar( a_source, a_eDataType, a_component );

	return 0.0f;
}

// $deBlob: FUNCTION 007e58a0
TFLOAT ANEDKeyframeInterpolation::GetRootScalar( const TransformKeyData* a_pKey, DataType a_eDataType, TINT a_iComponent )
{
	if ( a_eDataType == DataType_Transform )
	{
		switch ( a_iComponent )
		{
			case KeyComponent_TranslationX: return a_pKey->pMatrix->m_f41;
			case KeyComponent_TranslationY: return a_pKey->pMatrix->m_f42;
			case KeyComponent_TranslationZ: return a_pKey->pMatrix->m_f43;
			case KeyComponent_ScaleX: return a_pKey->pMatrix->AsBasisVector4( BASISVECTOR_RIGHT ).Magnitude();
			case KeyComponent_ScaleY: return a_pKey->pMatrix->AsBasisVector4( BASISVECTOR_UP ).Magnitude();
			case KeyComponent_ScaleZ: return a_pKey->pMatrix->AsBasisVector4( BASISVECTOR_FORWARD ).Magnitude();
		}
	}

	return 0.0f;
}

// $deBlob: FUNCTION 007e5940
TINT ANEDKeyframeInterpolation::FindKey( const KeyList* a_pKeyList, TFLOAT a_fTime )
{
	for ( TINT i = 0; i < a_pKeyList->iNumKeys; i++ )
	{
		if ( a_pKeyList->pKeys[ i ].fTime <= a_fTime && ( i + 1 >= a_pKeyList->iNumKeys || a_fTime < a_pKeyList->pKeys[ i + 1 ].fTime ) )
			return i;
	}

	return -1;
}
