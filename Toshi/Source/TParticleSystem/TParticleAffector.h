#pragma once


TOSHI_NAMESPACE_START

class TGenericParticles;

// Changes the particles every update (moves them, drag, forces...), 0xC bytes, a list in the settings
class TParticleAffector
{
public:
	enum TYPE
	{
		TYPE_PHYSICS,
		TYPE_DRAG,
		TYPE_CAPPEDSPEEDDRAG,
		TYPE_FORCE,
		TYPE_RANDOMFORCE,
		TYPE_VELOCITYROTATE,
		TYPE_PUSHPULL,
		TYPE_ROTATEAROUNDEMITTERX,
		TYPE_ROTATEAROUNDEMITTERY,
		TYPE_ROTATEAROUNDEMITTERZ,
		TYPE_CUSTOM,
	};

	struct TRBHeader
	{
		TUINT m_uiUnk0; // 0x00
		TUINT m_eType;  // 0x04
		TUINT m_uiSize; // 0x08 (with the data that follows)
	};

	using t_UpdateFn = void( __stdcall* )( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TRBHeader* a_pData );

public:
	TParticleAffector( const TParticleAffector& a_rcOrig );
	TParticleAffector( TRBHeader* a_pData );
	~TParticleAffector();

	void Create();

	// $deBlob: FUNCTION 007198e0
	void Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime ) { m_fnUpdate( a_pParticles, a_iStart, a_iEnd, a_fDeltaTime, m_pData ); }

	template <class T>
	T* GetData() const { return TSTATICCAST( T, m_pData ); }

public:
	TRBHeader*         m_pData;    // 0x00
	TParticleAffector* m_pNext;    // 0x04
	t_UpdateFn         m_fnUpdate; // 0x08
};

class TParticleAffectorPhysics
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorDrag
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorCappedSpeedDrag
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorForce
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorRandomForce
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorVelocityRotate
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorPushPull
{
public:
	static void __stdcall Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

class TParticleAffectorRotateAroundEmitter
{
public:
	// The particles are moved with the emitter's transform: rotate around the local axis
	static void __stdcall UpdateX( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
	static void __stdcall UpdateY( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
	static void __stdcall UpdateZ( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
	// The particles are in world space: rotate around the emitter's position
	static void __stdcall UpdateWorldX( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
	static void __stdcall UpdateWorldY( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
	static void __stdcall UpdateWorldZ( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );

	// The game adds its own (APlayerEffectInstantDeath::AddCustomAffector)
	static void __stdcall UpdateCustom( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData );
};

TOSHI_NAMESPACE_END
