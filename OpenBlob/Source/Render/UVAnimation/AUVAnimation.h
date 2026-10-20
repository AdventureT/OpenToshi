#pragma once
#include <Toshi/T2DList.h>

class AUVAnimationPool;

//-----------------------------------------------------------------------------
// Purpose: the UV animation of a material pass, taken from an AUVAnimationPool by AMaterialManager
//-----------------------------------------------------------------------------
class AUVAnimation
    : public Toshi::T2DList<AUVAnimation>::Node
{
public:
	enum TYPE
	{
		TYPE_UVOFFSET = 0,
		TYPE_SPRITE   = 1, // a sprite sheet (asserts: "m_Type==Sprite")
		TYPE_NONE     = 2, // not updated (AMaterialManager::UpdateUVs skips it)
		TYPE_UNSET    = 3, // the constructor's
	};

	friend class AUVAnimationPool;

public:
	AUVAnimation();
	~AUVAnimation() = default;

	// A UV offset animation (UV function type 1)
	void SetupOffset();

	// UV function types 2 and 3 (a_bUnk: type 3)
	void SetupUnk1( TBOOL a_bUnk );

	// TYPE_NONE
	void SetupNone();

	// Updates the animation at a_fTime (the material manager's time) and returns the pass's UV offset; a_bUseCache
	// returns the last result when it was already updated this frame (UV offsets only)
	void Update( TUINT32 a_uiUFunc, TUINT32 a_uiVFunc, TFLOAT a_fUParam1, TFLOAT a_fUParam2, TFLOAT a_fVParam1, TFLOAT a_fVParam2, TFLOAT& a_rfUOffset, TFLOAT& a_rfVOffset, TFLOAT a_fTime, TINT a_iFrame, TBOOL a_bUseCache );

	TYPE GetType() const { return m_eType; }

	AUVAnimationPool* GetOwnerPool() const { return m_pOwnerPool; }

private:
	void UpdateUVOffset( TUINT32 a_uiUFunc, TUINT32 a_uiVFunc, TFLOAT a_fUParam1, TFLOAT a_fUParam2, TFLOAT a_fVParam1, TFLOAT a_fVParam2, TFLOAT& a_rfUOffset, TFLOAT& a_rfVOffset, TFLOAT a_fTime, TINT a_iFrame, TBOOL a_bUseCache );
	void UpdateSprite( TINT a_iNumColumns, TINT a_iNumRows, TFLOAT a_fFPS, TFLOAT a_fUnused1, TFLOAT a_fUnused2, TFLOAT a_fUnused3, TFLOAT& a_rfUOffset, TFLOAT& a_rfVOffset, TFLOAT a_fTime, TUINT a_uiFrame );

	TBOOL  EvaluateFunction( TUINT32 a_uiFunc, TFLOAT a_fParam1, TFLOAT& a_rfParam2, TFLOAT a_fTime );
	TFLOAT Sawtooth( TFLOAT a_fTime, TFLOAT a_fFrequency );
	TFLOAT Triangle( TFLOAT a_fTime, TFLOAT a_fPeriod, TFLOAT a_fAmplitude );
	// The sine and square waves
	TFLOAT Sine( TFLOAT a_fTime, TFLOAT a_fPeriod, TFLOAT a_fAmplitude );
	TFLOAT Square( TFLOAT a_fTime, TFLOAT a_fPeriod, TFLOAT a_fAmplitude );

private:
	AUVAnimationPool* m_pOwnerPool; // 0x08
	union
	{
		TFLOAT m_fUOffset; // 0x0C TYPE_UVOFFSET
		TBOOL  m_bUnk0C;   // 0x0C TYPE_SPRITE: random frames
	};
	union
	{
		TFLOAT m_fVOffset;    // 0x10 TYPE_UVOFFSET
		TFLOAT m_fSpriteTime; // 0x10 TYPE_SPRITE: the time not yet turned into frames
	};
	union
	{
		TFLOAT m_fUnk14;       // 0x14
		TINT   m_iSpriteFrame; // 0x14 TYPE_SPRITE
	};
	TFLOAT m_fUnk18;     // 0x18
	TYPE   m_eType;      // 0x1C
	TINT   m_iLastFrame; // 0x20 the frame of the last update
};                       // 0x24

//-----------------------------------------------------------------------------
// Purpose: a fixed number of AUVAnimations
//-----------------------------------------------------------------------------
class AUVAnimationPool
{
public:
	AUVAnimationPool( TINT a_iSize, const TCHAR* a_szName );
	~AUVAnimationPool();

	// The name is the cell's (ACellBucket::OnLoaded)
	void SetName( const TCHAR* a_szName ) { m_szName = a_szName; }

	TBOOL IsUsedEmpty() const { return m_lUsed.IsEmpty(); }

	// Moves a free animation to the used list
	AUVAnimation* Alloc();

	// Gives a_pUVAnim back to its pool
	static void Free( AUVAnimation* a_pUVAnim );

private:
	const TCHAR*                 m_szName;      // 0x00
	TINT                         m_iSize;       // 0x04
	AUVAnimation*                m_pAnimations; // 0x08
	Toshi::T2DList<AUVAnimation> m_lUsed;       // 0x0C
	Toshi::T2DList<AUVAnimation> m_lFree;       // 0x14
};                                              // 0x1C
