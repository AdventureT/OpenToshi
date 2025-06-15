#pragma once

TOSHI_NAMESPACE_START

namespace TMath
{
TBOOL ConeVsSphere(const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, TFLOAT a_fConeFOV, TFLOAT a_fSphereRadius, const TVector4& a_rvSpherePosition);
};

TOSHI_NAMESPACE_END
