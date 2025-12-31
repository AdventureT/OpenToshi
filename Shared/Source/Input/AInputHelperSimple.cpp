#include "pch.h"
#include "AInputHelperSimple.h"
#include "Toshi/Input/TInputDeviceKeyboard.h"

using namespace Toshi;

// $deBlob: FUNCTION 005dab60
void AInputHelperSimple::AddDoodad(size_t doodad)
{
	TASSERT(0 != m_iArraySize);
	TASSERT(m_iArrayIndex < m_iArraySize);

	m_pArray[m_iArrayIndex].m_iDoodad = doodad;
	m_iArrayIndex++;
}

// $deBlob: FUNCTION 005dace0
TBOOL AInputHelperSimple::IsHeld(size_t doodad)
{
	for (size_t i = 0; i < m_iArrayIndex; i++)
	{
		if (m_pArray[i].m_iDoodad == doodad)
		{
			return m_pArray[i].m_iState == STATE_HELD;
		}
	}
	return TFALSE;
}

// $deBlob: FUNCTION 005daca0
TBOOL AInputHelperSimple::WasPressed(size_t doodad)
{
	for (size_t i = 0; i < m_iArrayIndex; i++)
	{
		if (m_pArray[i].m_iDoodad == doodad)
		{
			return m_pArray[i].m_iState == STATE_PRESSED;
		}
	}
	return TFALSE;
}

// $deBlob: FUNCTION 005dabe0
void AInputHelperSimple::Update()
{
	if (m_pInputDevice != TNULL)
	{
		if (!m_pInputDevice->GetClass()->IsA(TGetClass(TInputDeviceKeyboard)) ||
		    m_pInputDevice->IsEnabled())
		{
			for (size_t i = 0; i < m_iArrayIndex; i++)
			{
				switch (m_pArray[i].m_iState)
				{
					case STATE_NONE:
						if (m_pInputDevice->IsDown(m_pArray[i].m_iDoodad))
						{
							m_pArray[i].m_iState = STATE_PRESSED;
						}
						break;
					case STATE_PRESSED:
						if (m_pInputDevice->IsDown(m_pArray[i].m_iDoodad))
						{
							m_pArray[i].m_iState = STATE_HELD;
						}
						else
						{
							m_pArray[i].m_iState = STATE_RELEASED;
						}
						break;
					case STATE_HELD:
						if (!m_pInputDevice->IsDown(m_pArray[i].m_iDoodad))
						{
							m_pArray[i].m_iState = STATE_RELEASED;
						}
						break;
					case STATE_RELEASED:
						m_pArray[i].m_iState = STATE_NONE;
						break;
				}
			}
		}
	}
}
