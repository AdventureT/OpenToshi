#include "ToshiPCH.h"
#include "TTransformObject.h"
#include "TRender.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006ab880
// $Barnyard: FUNCTION 006c8340
TTransformObject::TTransformObject()
{
	m_EulerOrder[ 0 ] = EULERAXIS_Z;
	m_EulerOrder[ 1 ] = EULERAXIS_Y;
	m_EulerOrder[ 2 ] = EULERAXIS_X;
	m_bChanged        = TTRUE;

	SetMatrix( TMatrix44::IDENTITY );
	SetTranslate( TVector3::VEC_ZERO );
	m_Scale = TVector3( 1.0f, 1.0f, 1.0f );
}

// $deBlob: FUNCTION 006ab930
TTransformObject::TTransformObject( const TTransformObject& a_rcOther )
    : TTransformData( a_rcOther )
{
	m_bChanged = TTRUE;
}

TTransformObject::~TTransformObject()
{
}

// $deBlob: FUNCTION 006ab9a0
// $Barnyard: FUNCTION 006c8180
TTransformObject& TTransformObject::operator=( const TTransformObject& a_rcOther )
{
	if ( &a_rcOther != this )
	{
		// NOTE: copies everything up to the dirty flag, the cached matrix is rebuilt
		TUtil::MemCopy( this, &a_rcOther, offsetof( TTransformObject, m_bChanged ) );
		m_bChanged = TTRUE;
	}

	return *this;
}

// $deBlob: FUNCTION 006ab9f0
// $Barnyard: FUNCTION 006c83b0
void TTransformObject::Push()
{
	TRender* pRender = TRender::GetSingleton();

	TMatrix44* pPrevTransform = &pRender->GetTransforms().Top();
	pRender->GetTransforms().PushNull();
	TMatrix44* pPushTransform = &pRender->GetTransforms().Top();

	if ( m_eMode == Mode::Quat )
	{
		pPushTransform->PushQuaternion( GetQuat(), *pPrevTransform, m_Translation );
	}
	else if ( m_eMode == Mode::Euler )
	{
		*pPushTransform = *pPrevTransform;
		TMatrix44::TransformVector( pPushTransform->GetTranslation().AsVector3(), *pPushTransform, GetTranslation() );

		for ( TINT i = 0; i < 3; i++ )
		{
			switch ( m_EulerOrder[ i ] )
			{
				case EULERAXIS_X:
					RotateX( *pPushTransform, m_eMode, GetEuler().x );
					break;
				case EULERAXIS_Y:
					RotateY( *pPushTransform, m_eMode, GetEuler().y );
					break;
				case EULERAXIS_Z:
					RotateZ( *pPushTransform, m_eMode, GetEuler().z );
					break;
			}
		}
	}
	else
	{
		TASSERT( Mode::Matrix == GetMode() );
		pPushTransform->Multiply( *pPrevTransform, m_Matrix );
	}

	// NOTE: de Blob scales in every mode (Barnyard doesn't in matrix mode)
	pPushTransform->Scale( m_Scale.x, m_Scale.y, m_Scale.z );

	pRender->GetCurrentContext()->SetModelViewMatrix( pRender->GetTransforms().Top() );
	m_bChanged = TTRUE;
}

// $deBlob: FUNCTION 006abc20
// $Barnyard: FUNCTION 006c84c0
void TTransformObject::Pop()
{
	TRender* pRender = TRender::GetSingleton();

	pRender->GetTransforms().Pop();
	pRender->GetCurrentContext()->SetModelViewMatrix( pRender->GetTransforms().Top() );
	m_bChanged = TTRUE;
}

TVector3& TTransformObject::GetTranslation()
{
	return ( m_eMode != Mode::Matrix ) ? m_Translation : m_Matrix.GetTranslation().AsVector3();
}

// $deBlob: FUNCTION 006abda0
// $Barnyard: FUNCTION 006c8250
void TTransformObject::GetLocalMatrixImp( TMatrix44& a_rOutMatrix )
{
	if ( m_eMode == Mode::Quat )
	{
		a_rOutMatrix.SetFromQuaternion( GetQuat() );
		a_rOutMatrix.SetTranslation( GetTranslation() );
	}
	else if ( m_eMode == Mode::Euler )
	{
		a_rOutMatrix.Identity();
		a_rOutMatrix.SetTranslation( GetTranslation() );

		for ( TINT i = 0; i < 3; i++ )
		{
			switch ( m_EulerOrder[ i ] )
			{
				case EULERAXIS_X:
					RotateX( a_rOutMatrix, m_eMode, GetEuler().x );
					break;
				case EULERAXIS_Y:
					RotateY( a_rOutMatrix, m_eMode, GetEuler().y );
					break;
				case EULERAXIS_Z:
					RotateZ( a_rOutMatrix, m_eMode, GetEuler().z );
					break;
			}
		}
	}
	else
	{
		TASSERT( Mode::Matrix == GetMode() );
		a_rOutMatrix = m_Matrix;
	}

	// NOTE: de Blob scales in every mode (Barnyard doesn't in matrix mode)
	a_rOutMatrix.Scale( m_Scale.x, m_Scale.y, m_Scale.z );
}

// $deBlob: FUNCTION 006abc90
// $Barnyard: FUNCTION 006c81b0
void TTransformObject::SetEuler( const TVector3& a_rEuler )
{
	m_bChanged = TTRUE;
	m_eMode    = Mode::Euler;
	GetEuler() = a_rEuler;
	m_bChanged = TTRUE;
}

// $deBlob: FUNCTION 006abd80
void TTransformObject::SetEulerOrder( EULERAXIS x, EULERAXIS y, EULERAXIS z )
{
	m_EulerOrder[ 0 ] = x;
	m_EulerOrder[ 1 ] = y;
	m_EulerOrder[ 2 ] = z;
}

// $deBlob: FUNCTION 006abd40
// $Barnyard: FUNCTION 006c8210
void TTransformObject::SetTranslate( const TVector3& a_rTranslation )
{
	m_Translation = a_rTranslation;

	if ( m_eMode == Mode::Matrix )
		m_Matrix.SetTranslation( a_rTranslation );

	m_bChanged = TTRUE;
}

// $deBlob: FUNCTION 006abcc0
void TTransformObject::SetQuat( const TQuaternion& quaternion )
{
	m_bChanged = TTRUE;
	m_eMode    = Mode::Quat;
	GetQuat()  = quaternion;
	m_bChanged = TTRUE;
}

// $deBlob: FUNCTION 006abcf0
// $Barnyard: FUNCTION 006c81d0
void TTransformObject::SetMatrix( const TMatrix44& matrix )
{
	m_bChanged    = TTRUE;
	m_eMode       = Mode::Matrix;
	m_Matrix      = matrix;
	m_Translation = matrix.GetTranslation3();
	m_bChanged    = TTRUE;
}

// $deBlob: FUNCTION 006abc70
// $Barnyard: FUNCTION 006c81a0
TTransformObject::Mode TTransformObject::SetMode( Mode a_eMode )
{
	Mode eOldMode = m_eMode;
	m_eMode       = a_eMode;
	m_bChanged    = TTRUE;

	return eOldMode;
}

void TTransformObject::SetScale( TFLOAT a_flScaleX, TFLOAT a_flScaleY, TFLOAT a_flScaleZ )
{
	m_Scale.x  = a_flScaleX;
	m_Scale.y  = a_flScaleY;
	m_Scale.z  = a_flScaleZ;
	m_bChanged = TTRUE;
}

void TTransformObject::SetScale( const TVector3& a_rScale )
{
	m_Scale    = a_rScale;
	m_bChanged = TTRUE;
}

TOSHI_NAMESPACE_END
