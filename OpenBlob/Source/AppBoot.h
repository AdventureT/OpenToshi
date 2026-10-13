#pragma once
#include <Toshi/TApplication.h>

class AApplication : public Toshi::TApplication
{
public:
	AApplication() {}
	~AApplication() {}

	virtual TBOOL OnCreate( TINT argc, TCHAR** argv ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual TBOOL OnDestroy() OVERRIDE;
};

extern AApplication g_oTheApp;
