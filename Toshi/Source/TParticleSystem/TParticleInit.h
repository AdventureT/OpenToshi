#pragma once
#include "TParticles.h"


TOSHI_NAMESPACE_START

class TGenericParticles;

// Initialises one buffer of new generic particles (positions, velocities, life, mass, size, UV index or colour),
// 8 bytes. The init functions of each type are in TParticleInitV4/Float/Int32/Colour.
class TParticleInit
{
public:
	enum TYPE
	{
		// TParticleInitV4 (TParticleV4Buffer)
		TYPEV4_FIXED,
		TYPEV4_MINMAX,
		TYPEV4_RANDDIR,
		TYPEV4_DIRTOWARDSCENTER,
		TYPEV4_DIRAWAYFROMCENTER,
		TYPEV4_DIRINCONE2,
		TYPEV4_DIRINCONE,
		TYPEV4_SPHEREAREA,
		TYPEV4_RANDDIR2,
		TYPEV4_CUBESURFACE,
		TYPEV4_CIRCLEAREA,
		TYPEV4_CIRCLESURFACE,
		TYPEV4_CONEAREA,
		TYPEV4_RANDDIRRANGE,
		TYPEV4_DIRTOWARDSCENTERRANGE,
		TYPEV4_DIRAWAYFROMCENTERRANGE,
		// TParticleInitFloat (TParticleFloatBuffer)
		TYPEFLOAT_FIXED,
		TYPEFLOAT_MINMAX,
		TYPEFLOAT_FROMSIZE,
		// TParticleInitInt32 (TParticleUIntBuffer)
		TYPEINT_FIXED,
		TYPEINT_MINMAX,
		// TParticleInitColour (TParticleUIntBuffer)
		TYPECOLOUR_FIXED,
		TYPECOLOUR_MINMAX,
	};

	struct TRBHeader
	{
		TUINT m_uiUnk0;  // 0x00
		TUINT m_eType;   // 0x04
		TUINT m_uiSize;  // 0x08 (with the data that follows)
	};

	using t_InitFn = void( __stdcall* )( void* a_pBuffer, TINT a_iStart, TINT a_iEnd, TRBHeader* a_pData );

public:
	TParticleInit( const TParticleInit& a_rcOrig );
	~TParticleInit();

	void Create();

	// $deBlob: FUNCTION 0071c5a0
	TBOOL IsTypeV4() const { return m_pData->m_eType < TYPEFLOAT_FIXED; }

	// $deBlob: FUNCTION 0071c5b0
	TBOOL IsTypeFloat() const { return m_pData->m_eType >= TYPEFLOAT_FIXED && m_pData->m_eType < TYPEINT_FIXED; }

	// $deBlob: FUNCTION 0071c5d0
	TBOOL IsTypeInt() const { return m_pData->m_eType >= TYPEINT_FIXED && m_pData->m_eType < TYPECOLOUR_FIXED; }

	// $deBlob: FUNCTION 0071c5f0
	TBOOL IsTypeColour() const { return m_pData->m_eType >= TYPECOLOUR_FIXED; }

	TUINT GetType() const { return m_pData->m_eType; }

	template <class T>
	T* GetData() const { return TSTATICCAST( T, m_pData ); }

	// $deBlob: FUNCTION 0071b110
	void Init( TParticleV4Buffer& a_rBuffer, TINT a_iStart, TINT a_iEnd )
	{
		TASSERT( IsTypeV4() );
		m_fnInit( &a_rBuffer, a_iStart, a_iEnd, m_pData );
	}

	// $deBlob: FUNCTION 0071b050
	void Init( TParticleFloatBuffer& a_rBuffer, TINT a_iStart, TINT a_iEnd )
	{
		TASSERT( IsTypeFloat() );
		m_fnInit( &a_rBuffer, a_iStart, a_iEnd, m_pData );
	}

	// $deBlob: FUNCTION 0071b0b0
	void InitInt( TParticleUIntBuffer& a_rBuffer, TINT a_iStart, TINT a_iEnd )
	{
		TASSERT( IsTypeInt() );
		m_fnInit( &a_rBuffer, a_iStart, a_iEnd, m_pData );
	}

	// $deBlob: FUNCTION 0071aff0
	void InitColour( TParticleUIntBuffer& a_rBuffer, TINT a_iStart, TINT a_iEnd )
	{
		TASSERT( IsTypeColour() );
		m_fnInit( &a_rBuffer, a_iStart, a_iEnd, m_pData );
	}

	// The particles being initialised (some init functions read their other buffers)
	// $deBlob: FUNCTION 0071b170
	static void SetCurParticles( TGenericParticles* a_pParticles ) { sm_pCurParticles = a_pParticles; }

public:
	static TGenericParticles* sm_pCurParticles;

public:
	t_InitFn   m_fnInit; // 0x00
	TRBHeader* m_pData;  // 0x04
};

// The init functions of the vectors (the data follows TParticleInit::TRBHeader)
class TParticleInitV4
{
public:
	// The faces of the box InitCubeSurface emits from
	enum CUBEFACE : TINT
	{
		CUBEFACE_PLUSZ,
		CUBEFACE_MINUSZ,
		CUBEFACE_PLUSY,
		CUBEFACE_MINUSY,
		CUBEFACE_PLUSX,
		CUBEFACE_MINUSX,
		CUBEFACE_NUMOF,
	};

	// TYPEV4_FIXED
	struct FixedData : TParticleInit::TRBHeader
	{
		TVector3 m_vValue; // 0x0C
	};

	// TYPEV4_MINMAX
	struct MinMaxData : TParticleInit::TRBHeader
	{
		TVector3 m_vMin;   // 0x0C
		TFLOAT   m_fUnk18; // 0x18
		TVector3 m_vMax;   // 0x1C
	};

	static void __stdcall InitFixed( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitMinMax( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitRandDir( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitDirTowardsCenter( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitDirAwayFromCenter( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitDirInCone2( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitDirInCone( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitRandDirRange( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitDirTowardsCenterRange( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitDirAwayFromCenterRange( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitSphereArea( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitCubeSurface( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitCircleSurface( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitCircleArea( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitConeArea( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
};

// The init functions of the floats (life lost per second, mass, size)
class TParticleInitFloat
{
public:
	static void __stdcall InitFixed( TParticleFloatBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitMinMax( TParticleFloatBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitFromSize( TParticleFloatBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
};

// The init functions of the integers (UV index)
class TParticleInitInt32
{
public:
	static void __stdcall InitFixed( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitMinMax( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
};

// The init functions of the colours
class TParticleInitColour
{
public:
	// TYPECOLOUR_FIXED
	struct FixedData : TParticleInit::TRBHeader
	{
		TUINT32 m_uiColour; // 0x0C

		void ConvertColourToSKUColour();
	};

	// TYPECOLOUR_MINMAX
	struct MinMaxData : TParticleInit::TRBHeader
	{
		TUINT8 m_aMin[ 4 ]; // 0x0C
		TUINT8 m_aMax[ 4 ]; // 0x10

		void ConvertColourToSKUColour();
	};

	static void __stdcall InitFixed( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
	static void __stdcall InitMinMax( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData );
};

TOSHI_NAMESPACE_END
