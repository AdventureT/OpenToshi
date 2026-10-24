#pragma once
#include "Math/TMatrix44.h"
#include "Math/TQuaternion.h"
#include "Math/TVector3.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the transform data of a TTransformObject (its first 0x60 bytes)
//-----------------------------------------------------------------------------
struct TTransformData
{
	enum class Mode
	{
		Quat = 0,
		Euler,
		Matrix
	};

	using EULERAXIS = TUINT8;
	enum EULERAXIS_ : EULERAXIS
	{
		EULERAXIS_X = 0,
		EULERAXIS_Y = 1,
		EULERAXIS_Z = 2,
	};

	TTransformData() {}

	// $deBlob: FUNCTION 006abfc0
	TTransformData( const TTransformData& a_rcOther )
	{
		m_Matrix          = a_rcOther.m_Matrix;
		m_eMode           = a_rcOther.m_eMode;
		m_Translation     = a_rcOther.m_Translation;
		m_Scale           = a_rcOther.m_Scale;
		m_EulerOrder[ 0 ] = a_rcOther.m_EulerOrder[ 0 ];
		m_EulerOrder[ 1 ] = a_rcOther.m_EulerOrder[ 1 ];
		m_EulerOrder[ 2 ] = a_rcOther.m_EulerOrder[ 2 ];
		m_uiPad5F         = a_rcOther.m_uiPad5F;
	}

	// The rotation as a matrix, quaternion or euler angles (m_eMode). A union of the three in the original; here the
	// matrix's storage with the other two as views, as VS2013 has no unions of classes with copy constructors
	TQuaternion&       GetQuat() { return *TREINTERPRETCAST( TQuaternion*, &m_Matrix ); }
	const TQuaternion& GetQuat() const { return *TREINTERPRETCAST( const TQuaternion*, &m_Matrix ); }
	TVector3&          GetEuler() { return *TREINTERPRETCAST( TVector3*, &m_Matrix ); }
	const TVector3&    GetEuler() const { return *TREINTERPRETCAST( const TVector3*, &m_Matrix ); }

	TMatrix44 m_Matrix; // 0x00

	Mode      m_eMode;           // 0x40
	TVector3  m_Translation;     // 0x44
	TVector3  m_Scale;           // 0x50
	EULERAXIS m_EulerOrder[ 3 ]; // 0x5C
	TUINT8    m_uiPad5F;         // 0x5F copied with the euler order
}; // 0x60

//-----------------------------------------------------------------------------
// Purpose: local transform of a scene object. de Blob's version has a dirty flag (+0x60) and caches the local
// matrix (+0x64), 0xA4 bytes in all
//-----------------------------------------------------------------------------
class TTransformObject
    : protected TTransformData
{
public:
	TTransformObject();
	TTransformObject( const TTransformObject& a_rcOther );
	~TTransformObject();

	TTransformObject& operator=( const TTransformObject& a_rcOther );

	void Push();
	void Pop();

	TVector3& GetTranslation();
	void      GetLocalMatrixImp( TMatrix44& a_rOutMatrix );

	void SetEuler( const TVector3& a_rEuler );
	void SetEulerOrder( EULERAXIS a_uiX, EULERAXIS a_uiy, EULERAXIS a_uiz );
	void SetTranslate( const TVector3& a_rTranslation );
	void SetQuat( const TQuaternion& a_rQuaternion );
	void SetMatrix( const TMatrix44& a_rMatrix );
	Mode SetMode( Mode a_eMode );
	void SetScale( TFLOAT a_flScaleX, TFLOAT a_flScaleY, TFLOAT a_flScaleZ );
	void SetScale( const TVector3& a_rScale );

	// $deBlob: FUNCTION 00554d90
	TMatrix44 GetLocalMatrix()
	{
		if ( m_bChanged )
		{
			GetLocalMatrixImp( m_LocalMatrix );
			m_bChanged = TFALSE;
		}

		return m_LocalMatrix;
	}

	// $deBlob: FUNCTION 00554de0
	TMatrix44& GetMatrix()
	{
		TASSERT( Mode::Matrix == GetMode() );
		return m_Matrix;
	}

	const TMatrix44& GetMatrix() const
	{
		TASSERT( Mode::Matrix == GetMode() );
		return m_Matrix;
	}

	void ResetChangedState()
	{
		m_bChanged = TFALSE;
	}

	TBOOL HasChanged() const
	{
		return m_bChanged;
	}

	// $deBlob: FUNCTION 00554e30
	Mode GetMode() const
	{
		return m_eMode;
	}

	const TVector3& GetScale() const
	{
		return m_Scale;
	}

private:
	static void RotateX( TMatrix44& a_rMatrix, Mode a_eMode, TFLOAT a_fAngle )
	{
		TASSERT( Mode::Euler == a_eMode );
		a_rMatrix.RotateX( a_fAngle );
	}

	static void RotateY( TMatrix44& a_rMatrix, Mode a_eMode, TFLOAT a_fAngle )
	{
		TASSERT( Mode::Euler == a_eMode );
		a_rMatrix.RotateY( a_fAngle );
	}

	static void RotateZ( TMatrix44& a_rMatrix, Mode a_eMode, TFLOAT a_fAngle )
	{
		TASSERT( Mode::Euler == a_eMode );
		a_rMatrix.RotateZ( a_fAngle );
	}

private:
	TBOOL     m_bChanged;        // 0x60 the local matrix needs to be rebuilt
	TMatrix44 m_LocalMatrix;     // 0x64 cached by GetLocalMatrix
}; // 0xA4

TOSHI_NAMESPACE_END
