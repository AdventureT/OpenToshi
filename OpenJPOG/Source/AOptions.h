#pragma once
#include "Toshi/Plugins/PPropertyParser/PPropertyReader.h"


class AOptionChangeEvent
{

};

class AOptionLoadEvent
{

};

class AOptions : public Toshi::TSingleton<AOptions>
{

	enum Result
	{
		RESULT_OK = 0,
		RESULT_ERROR = 3,
	};

public:

	AOptions() : m_oOptionLoadEmitter(this), m_oOptionChangeEmitter(this), m_iAutoSaveState(1)
	{
		PProperties* props = new PProperties();
		m_pUnkProps = props;
		m_pCurProps = props;
	}

	Result LoadOptions();

	static AOptions* GetOptions() { return GetSingleton(); }

	int GetAutoSaveState() const { return m_iAutoSaveState; }
	PProperties* GetCurrentProperties() { return m_pCurProps; }
	Toshi::TEmitter<AOptions, AOptionLoadEvent>* GetOptionLoadEmitter() { return &m_oOptionLoadEmitter; }
	Toshi::TEmitter<AOptions, AOptionChangeEvent>* GetOptionChangeEmitter() { return &m_oOptionChangeEmitter; }

protected:
	Result LoadOptions(int a_iUnk, int a_iUnk2, const Toshi::TCString& a_sStr1, const Toshi::TCString& a_sStr2);

private:

	static const int sm_iSlot = -1;
	static const int sm_iPort = -1;
	static constexpr const char* sm_szOptionsDir = TNULL;
	static constexpr const char* sm_szOptionsName = "Options";

	Toshi::TEmitter<AOptions, AOptionLoadEvent>   m_oOptionLoadEmitter;   // 0x4
	Toshi::TEmitter<AOptions, AOptionChangeEvent> m_oOptionChangeEmitter; // 0x10
	int m_iAutoSaveState;                                                 // 0x1C
	PProperties* m_pUnkProps;                                             // 0x20
	PProperties* m_pCurProps;                                             // 0x24
};

