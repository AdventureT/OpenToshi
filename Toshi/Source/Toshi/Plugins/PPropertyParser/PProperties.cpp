#include "ToshiPCH.h"
#include "PProperties.h"




const PPropertyValue* PProperties::GetProperty(const Toshi::TPCString& a_szPropertyName)
{
    PPropertyName name = PPropertyName();
    return nullptr;
}

const PPropertyValue* PProperties::GetProperty(const PPropertyName& a_szPropertyName) const
{
    for (auto i = m_oProps.Begin(); i != m_oProps.End(); i++)
    {
        if (i->GetName() == a_szPropertyName)
        {
            return i->GetValue();
        }
    }
    return TNULL;
}

const PPropertyName& PProperties::PProperty::GetName() const
{
    return m_oName;
}

inline int PProperties::PProperty::GetLine() const
{
    return m_iLine;
}

inline const PPropertyValue* PProperties::PProperty::GetValue() const
{
    return &m_oValue;
}
