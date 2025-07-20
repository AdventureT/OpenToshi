#pragma once
class AOptions : Toshi::TObject
    , public Toshi::TSingleton<AOptions>
{
public:
	AOptions()
	{
		TTODO("Actually read it from g_oTheApp, aka safefile");
		m_Options->m_iUnk    = 2;
		m_Options->m_iPitch  = 8;
		m_Options->m_iVolume = 7;
	}

	typedef TUINT32 OPTIONS;
	enum OPTIONS_ : OPTIONS
	{
		OPTIONS_MAXSIZE = 28
	};

	// This is a part of the savefile!

	struct Options
	{
		TINT   m_iUnk;
		TINT   m_iPitch;
		TINT   m_iVolume;
		TFLOAT m_fUnk;
		TINT   m_iUnk2;
		TINT   m_iUnk3;
		TINT   m_iUnk4;
	};
	static_assert(sizeof(Options) == OPTIONS_MAXSIZE);

	Options* m_Options; // 0x4

	void   SoundThings(TBOOL unk, TFLOAT fUnk, TFLOAT fUnk2);
	TFLOAT GetPitch();

	TBOOL GetOptions(void* a_Options, TUINT32 a_iMaxSize)
	{
		TASSERT(OPTIONS_MAXSIZE <= a_iMaxSize);
		Toshi::TUtil::MemCopy(a_Options, m_Options, OPTIONS_MAXSIZE);
	}
};
