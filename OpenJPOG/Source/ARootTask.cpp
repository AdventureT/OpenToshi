#include "pch.h"
#include "ARootTask.h"

ARootTask::ARootTask()
{

}

bool ARootTask::OnUpdate(float deltaTime)
{
	return true;
}


bool ARootTask::OnCreate()
{
	TIMPLEMENT_D("APrim::InitialiseAllPrimStatics()");
	TIMPLEMENT_D("AOptionLogic::AOptionLogic()");
	TIMPLEMENT_D("AOptionLogic::EnumerateModes()");
	TIMPLEMENT_D("AOptions::LoadOptions(m_oOptions)");
	TIMPLEMENT_D("DeserialiseOptions()");
	TIMPLEMENT_D("SerialiseOptions()");
	TIMPLEMENT_D("InitialiseConstants()");

	m_oOptions.LoadOptions();

	return false;
}

bool ARootTask::CreateRenderInterface()
{
	return false;
}
