#include "ToshiPCH.h"
#include "TPlane.h"
#include "TMath.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0068ba30
// $deBlob: FUNCTION 0068ba80
void TPlane::Set( const TVector3& a_rcPoint1, const TVector3& a_rcPoint2, const TVector3& a_rcPoint3 )
{
	TMath::TriangleNormalCounterClockwise( m_Normal, a_rcPoint1, a_rcPoint2, a_rcPoint3 );
	m_fDistance = a_rcPoint1.y * m_Normal.y + a_rcPoint1.x * m_Normal.x + a_rcPoint1.z * m_Normal.z;
}

TOSHI_NAMESPACE_END
