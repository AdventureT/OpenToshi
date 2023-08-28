#pragma once
//#include <Toshi/Plugins/PPropertyParser/PProperties.h>

class PPropertyValue
{
public:
	static const Toshi::TClass* TYPE_PROPS;
	static inline const Toshi::TClass* TYPE_UNDEF = TNULL;

	PPropertyValue()
	{
		
	}

	void SetTObject(Toshi::TObject* a_pObject)
	{
		if (m_pTObject)
		{
			ChangeType(a_pObject->GetClass());
		}
		else
		{
			ChangeType(TYPE_UNDEF);
		}
		m_pTObject = a_pObject;
	}

protected:
	TBOOL ChangeType(const Toshi::TClass* a_pClass);

private:
	Toshi::TObject* m_pTObject;
};