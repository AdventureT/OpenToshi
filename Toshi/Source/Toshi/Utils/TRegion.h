#pragma once

TOSHI_NAMESPACE_START

class TRegion
{
public:
	// $deBlob: FUNCTION 00662d20
	static TINT GetRegion() { return s_ToshiRegion; }
	// $deBlob: FUNCTION 00662cd0
	static void SetRegion(TINT region)
	{
		TASSERT(!s_bRegionLocked);
		s_ToshiRegion = region;
	}
	static void LockRegion() { s_bRegionLocked = TTRUE; }
	static void UnlockRegion() { s_bRegionLocked = TFALSE; }

private:
	static TBOOL s_bRegionLocked;
	static TINT  s_ToshiRegion;
};

TOSHI_NAMESPACE_END
