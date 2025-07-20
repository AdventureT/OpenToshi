#pragma once
#include "AResourceView.h"
#include <Plugins/PPropertyParser/PBProperties.h>

class APropertiesResourceView : public AResourceView
{
public:
	APropertiesResourceView(AResourceFilePtr a_pResourceFile)
	    : AResourceView(a_pResourceFile), m_Properties(*PBProperties::LoadFromTRB(a_pResourceFile->GetTRB()))
	{
		static int s_iGlobalId = 0;

		m_bInitialised           = TFALSE;
		m_pSelectedProperty      = TNULL;
		m_pSelectedValue         = TNULL;
		m_PropertyNameBuffer[0]  = '\0';
		m_PropertyValueBuffer[0] = '\0';

		Toshi::TStringManager::String8Format(m_szIdListWindowName, 48, "PPropertyList%d", s_iGlobalId);
		Toshi::TStringManager::String8Format(m_szIdEditorWindowName, 48, "PPropertyEditor%d", s_iGlobalId);
		Toshi::TStringManager::String8Format(m_szIdPropertyName, 48, "##PropertyName%d", s_iGlobalId);
		Toshi::TStringManager::String8Format(m_szIdPropertyValue, 48, "##PropertyValue%d", s_iGlobalId);
		s_iGlobalId++;
	}

	~APropertiesResourceView()
	{
		TOSHI_INFO("Destroying APropertiesResourceView");
	}

	virtual void Render() override;

	virtual TBOOL SupportsSave() override { return TTRUE; }
	virtual void  Save() override;

	PBProperties&       GetProperties() { return m_Properties; }
	const PBProperties& GetProperties() const { return m_Properties; }

private:
	void ShowProperties(PBProperties* a_pProperties);

private:
	PBProperties              m_Properties;
	PBProperties::PBProperty* m_pSelectedProperty;
	PBPropertyValue*          m_pSelectedValue;
	char                      m_PropertyNameBuffer[512];
	char                      m_PropertyValueBuffer[512];
	TBOOL                     m_bInitialised;
	char                      m_szIdListWindowName[48];
	char                      m_szIdEditorWindowName[48];
	char                      m_szIdPropertyName[48];
	char                      m_szIdPropertyValue[48];
};
