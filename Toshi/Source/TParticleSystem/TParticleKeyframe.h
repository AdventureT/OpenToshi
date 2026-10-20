#pragma once


TOSHI_NAMESPACE_START

class TGenericParticlesSettings;
class TGenericParticles;
struct TParticleKeyframeState;

class TParticleKeyframe
{
public:
	enum KEYFRAMETYPE
	{
		KEYFRAMETYPE_COL                  = 1,  // TParticleKeyframeColour (the colour of the particles)
		KEYFRAMETYPE_ALPHA                = 2,  // TParticleKeyframeColour (only the alpha)
		KEYFRAMETYPE_MASS                 = 3,  // TParticleKeyframeMass
		KEYFRAMETYPE_SIZE                 = 5,  // TParticleKeyframeSize
		KEYFRAMETYPE_SIZEMULT             = 6,  // TParticleKeyframeSize (multiplies the size)
		KEYFRAMETYPE_UV                   = 7,  // TParticleKeyframeUV
		KEYFRAMETYPE_PARTICLESYSTEM       = 8,  // TKeyframeParticles (a particle system of an effect)
		KEYFRAMETYPE_REVERSEVELDIRS       = 9,  // TParticleKeyframeEventReverseVelDirs
		KEYFRAMETYPE_RANDOMISEVELDIRS     = 10, // TParticleKeyframeEventRandomiseVelDirs
	};

	enum LOOPTYPE
	{
		LOOPTYPE_NONE,
		LOOPTYPE_START,
		LOOPTYPE_END,
	};

	struct TRBHeader
	{
		TUINT   m_uiUnk0;    // 0x00
		TUINT16 m_uiEasing;  // 0x04
		TINT16  m_iLoopType; // 0x06
		TUINT   m_eType;     // 0x08
		TFLOAT  m_fTime;     // 0x0C
		TUINT   m_uiSize;    // 0x10
	};

	// KEYFRAMETYPE_COL, KEYFRAMETYPE_ALPHA
	struct ColourData : TRBHeader
	{
		TUINT32 m_uiColour; // 0x14 (the alpha, 0-255, for KEYFRAMETYPE_ALPHA)
	};

	// KEYFRAMETYPE_MASS, KEYFRAMETYPE_SIZE, KEYFRAMETYPE_SIZEMULT
	struct FloatData : TRBHeader
	{
		TFLOAT m_fValue; // 0x14
	};

	// KEYFRAMETYPE_UV
	struct UVData : TRBHeader
	{
		TFLOAT m_fUVIndex; // 0x14
	};

	// KEYFRAMETYPE_PARTICLESYSTEM (the keys of an effect's system timeline)
	struct ParticleSystemData : TRBHeader
	{
		TGenericParticlesSettings* m_pSettings;    // 0x14 (set at runtime by TParticleEffect::Create)
		TParticleKeyframe*         m_pPrevKey;     // 0x18 (set at runtime by TParticleEffect::Create)
		TFLOAT                     m_fNumParticles; // 0x1C (the share of the settings' particles)
	};

	// Called when the keyframe becomes the current one (a_pPrevKey: the one before it, if any)
	using t_StartFn = void( __stdcall* )( TParticleKeyframe* a_pKey, TParticleKeyframe* a_pPrevKey );
	// Called every update of an effect's system timeline while the keyframe is the current one
	using t_UpdateFn = void( __stdcall* )( TParticleKeyframe* a_pKey, TFLOAT a_fDeltaTime, TFLOAT a_fT, TParticleKeyframeState* a_pState );
	// Called for each particle of the generic particles whose life is in the keyframe
	using t_UpdateParticleFn = void( __stdcall* )( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );

public:
	TParticleKeyframe( const TParticleKeyframe& a_rcOrig );
	~TParticleKeyframe();

	void Link();

	TUINT  GetType() const { return m_pData->m_eType; }
	TFLOAT GetTime() const { return m_pData->m_fTime; }

	template <class T>
	T* GetData() const { return TSTATICCAST( T, m_pData ); }

	TFLOAT Ease( TFLOAT a_fT ) const;

	// $deBlob: FUNCTION 0071bc30
	void Start( TParticleKeyframe* a_pPrevKey ) { m_fnStart( this, a_pPrevKey ); }

	// $deBlob: FUNCTION 0071bc60
	void Update( TFLOAT a_fDeltaTime, TFLOAT a_fT, TParticleKeyframeState* a_pState ) { m_fnUpdate( this, a_fDeltaTime, a_fT, a_pState ); }

	// $deBlob: FUNCTION 0071bc90
	void UpdateParticle( TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle ) { m_fnUpdateParticle( this, a_pParticles, a_fT, a_iParticle ); }

	// TParticleKeyframeColour::GetColour (lerps from the particle's colour to the key's)
	TUINT32 GetColour( TGenericParticles* a_pParticles, TINT a_iParticle );
	// TParticleKeyframeColour::GetColour for KEYFRAMETYPE_ALPHA
	TUINT32 GetAlpha( TGenericParticles* a_pParticles, TINT a_iParticle );
	// TParticleKeyframeSize::GetSize
	TFLOAT GetSize( TGenericParticles* a_pParticles, TINT a_iParticle );

public:
	TRBHeader*         m_pData;            // 0x00
	TParticleKeyframe* m_pPrev;            // 0x04
	t_StartFn          m_fnStart;          // 0x08
	t_UpdateFn         m_fnUpdate;         // 0x0C
	t_UpdateParticleFn m_fnUpdateParticle; // 0x10
};

// The keyframe types (Link sets the callbacks of the keyframe)
class TParticleKeyframeColour
{
public:
	static void Link( TParticleKeyframe* a_pKey );
	static void LinkAlpha( TParticleKeyframe* a_pKey );

	static void __stdcall UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
};

class TParticleKeyframeMass
{
public:
	static void Link( TParticleKeyframe* a_pKey );

	static void __stdcall UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
};

class TParticleKeyframeSize
{
public:
	static void Link( TParticleKeyframe* a_pKey );

	static void __stdcall UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
};

class TParticleKeyframeUV
{
public:
	static void Link( TParticleKeyframe* a_pKey );

	static void __stdcall UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
};

// The keys of an effect's system timeline: create and feed the system's generic particles
class TKeyframeParticles
{
public:
	static void Link( TParticleKeyframe* a_pKey );

	static void __stdcall Update( TParticleKeyframe* a_pKey, TFLOAT a_fDeltaTime, TFLOAT a_fT, TParticleKeyframeState* a_pState );
};

class TParticleKeyframeEventReverseVelDirs
{
public:
	static void Link( TParticleKeyframe* a_pKey );

	static void __stdcall UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
};

class TParticleKeyframeEventRandomiseVelDirs
{
public:
	static void Link( TParticleKeyframe* a_pKey );

	static void __stdcall UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
};

// The default callbacks of the keyframes (a key type that doesn't need one keeps it)
void __stdcall Keyframe_DefaultStart( TParticleKeyframe* a_pKey, TParticleKeyframe* a_pPrevKey );
void __stdcall Keyframe_DefaultUpdate( TParticleKeyframe* a_pKey, TFLOAT a_fDeltaTime, TFLOAT a_fT, TParticleKeyframeState* a_pState );
void __stdcall Keyframe_DefaultUpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle );
void __stdcall Keyframe_SetPrevKey( TParticleKeyframe* a_pKey, TParticleKeyframe* a_pPrevKey );

// The playback state of a timeline (0x10 bytes), one per system of an effect instance
struct TParticleKeyframeState
{
	TINT16             m_iCurKey;  // 0x00
	TINT16             m_iLoops;   // 0x02
	TParticleKeyframe* m_pCurKey;  // 0x04
	TParticleKeyframe* m_pPrevKey; // 0x08
	TFLOAT             m_fTime;    // 0x0C
};

class TParticleKeyframeTimeline
{
public:
	TParticleKeyframeTimeline( const TParticleKeyframeTimeline& a_rcOrig );
	~TParticleKeyframeTimeline();

	static void InitState( TParticleKeyframeState& a_rState );

	void               Link();
	TParticleKeyframe* GetNextKeyframe( TParticleKeyframeState& a_rState );
	TBOOL              Update( TFLOAT a_fDeltaTime, TParticleKeyframeState& a_rState );
	TBOOL              Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleKeyframeState& a_rState );

	// Per particle (the keys are picked by the particles' life)
	void UpdateEvents( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd );
	void UpdateParticles( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd );
	void InitParticles( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd );

	TINT               GetNumKeyframes() const { return m_iNumLoopKeyframes; }
	TParticleKeyframe* GetKeyframe( TINT a_iIndex ) const { return m_aLoopKeyframes[ a_iIndex ]; }

public:
	TParticleKeyframe** m_aLoopKeyframes;    // 0x00
	TINT                m_iNumLoopKeyframes; // 0x04
};

TOSHI_NAMESPACE_END
