#include "ToshiPCH.h"
#include "TXUIResource.h"
#include "TXUIElement.h"
#include "TXUICanvas.h"
#include "Toshi/Xui/TXUIScene.h"
#include "Toshi/Xui/TXUI.h"
#include "TXUIGroup.h"
#include "TXUIBackButton.h"
#include "TXUINavButton.h"
#include "TXUIText.h"
#include "TXUITextPresenter.h"
#include "TXUIImage.h"
#include "TXUIImagePresenter.h"
#include "TXUIFigure.h"
#include "TXUINineGrid.h"
#include "XURReader.h"
#include "TXUILabel.h"
#include "TXUITabScene.h"
#include "TXUIScrollEnd.h"
#include "TXUISliderBar.h"
#include "TXUICheckBox.h"
#include "TXUICommonList.h"
#include "TXUIList.h"
#include "TXUIListItem.h"
#include "TXUIVideo.h"
#include "TXUIVisual.h"

TOSHI_NAMESPACE_START

TXUIResource::~TXUIResource()
{
	if (m_pMemoryHeap)
	{
		m_oHeader.m_apSections = TNULL;
		TMemory::DestroyHeap(m_pMemoryHeap);
	}
	else
	{
		for (size_t i = 0; i < m_uiStringCount; i++) delete[] m_asStringTable[i];

		delete[] m_asStringTable;
		delete[] m_avVectors;
		delete[] m_aqZRotTable;

		if (m_pRoot) delete m_pRoot;

		if (m_pStringTable) delete m_pStringTable;

		delete[] m_pCustData;
		delete[] m_oHeader.m_apSections;
	}
}

TBOOL TXUIResource::ReadHeader(TUINT8* buffer)
{
	XURReader reader(buffer);

	m_oHeader.m_uiFileID = reader.ReadUInt32LE();
	TASSERT(m_oHeader.m_uiFileID == IDXUR, "Not a .xur file!");

	m_oHeader.m_uiVersion     = reader.ReadUInt32();
	m_oHeader.m_uiFlags       = reader.ReadUInt16();
	m_oHeader.m_uiXuiVersion  = reader.ReadUInt32();
	m_oHeader.m_usBinSize     = reader.ReadUInt32();
	m_oHeader.m_usNumSections = reader.ReadUInt16();

	TASSERT(m_oHeader.m_usNumSections > 0, "There must be one or more Sections");
	m_oHeader.m_apSections = new (TXUI::MemoryBlock()) Section[m_oHeader.m_usNumSections];

	TUINT32 sectionID = PARSEDWORD(buffer);

	if (sectionID != IDXURSTRING && sectionID != IDXURVEC && sectionID != IDXURQUAT && sectionID != IDXURCUST) buffer += 0x28;

	for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
	{
		m_oHeader.m_apSections[i].m_uiSectionID = reader.ReadUInt32LE();

		TASSERT(
		    m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING || m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC || m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT || m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST ||
		        m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA,
		    "Invalid Section ID"
		);

		m_oHeader.m_apSections[i].m_uiOffset = reader.ReadUInt32();
		m_oHeader.m_apSections[i].m_uiSize   = reader.ReadUInt32();
	}

	return TTRUE;
}


void TXUIResource::Load(TBOOL loadStringTables, const TCHAR* filenameXUIB, const TCHAR* fileNameStringTable, TBOOL loadTrb, void* unk3)
{
	Destroy();

	if (loadStringTables && fileNameStringTable != TNULL)
	{
		TTODO("Load Stringtables");
	}

	const TCHAR* extension    = loadTrb ? ".trb" : ".xur";
	const TCHAR* fullFileName = strstr(filenameXUIB, extension);

	if (!loadTrb)
	{
		TFile* file = TFile::Create(fullFileName);

		if (file != TNULL)
		{
			TINT    size   = file->GetSize();
			TUINT8* buffer = new TUINT8[size];
			file->Read(buffer, size);
			file->Destroy();
			TBOOL bRes = ReadHeader(buffer);

			if (bRes)
			{
				for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
				{
					TTODO("Finish LoadXUIB2");
				}
			}
		}
	}
}

TBOOL TXUIResource::Load(TUINT8* buffer)
{
	Destroy();
	ReadHeader(buffer);

	TINT totalSize = m_oHeader.m_usNumSections * sizeof(Section) + 4;

	for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
	{
		auto&   section              = m_oHeader.m_apSections[i];
		TUINT8* currentSectionBuffer = buffer + section.m_uiOffset;

		if (section.m_uiSectionID == IDXURQUAT)
		{
			ReadQuatSection(currentSectionBuffer, section.m_uiSize);
			totalSize += (section.m_uiSize / sizeof(TQuaternion)) * 4 + 4;
		}
		else if (section.m_uiSectionID == IDXURCUST)
		{
			ReadCustSection(currentSectionBuffer, section.m_uiSize);
		}
		else if (section.m_uiSectionID == IDXURDATA)
		{
			ReadDataSection(currentSectionBuffer, section.m_uiSize);
		}
		else if (section.m_uiSectionID == IDXURSTRING)
		{
			ReadStringSection(currentSectionBuffer, section.m_uiSize);
			totalSize += GetStringTableSize(currentSectionBuffer, section.m_uiSize);
		}
		else if (section.m_uiSectionID == IDXURVEC)
		{
			totalSize += (section.m_uiSize / sizeof(TVector3)) * 8 + 0xC;
		}
	}

	return TTRUE;
}

TBOOL TXUIResource::ReadDataSection(TUINT8* a_pData, TUINT32 size)
{
	XURReader reader(a_pData);

	TUINT8* pValidateEnd = a_pData + size;
	TUINT16 uiType       = reader.ReadUInt16();

	TASSERT(0 == TStringManager::String16Compare(GetString(uiType), _TS16("XuiCanvas")), "First Element is not XuiCanvas!");
	m_pRoot = CreateObjectData(*this, uiType);

	TUINT8 opcode = reader.ReadUInt8();

	m_pRoot->Load(*this, a_pData);

	if (HASFLAG(opcode & 2))
	{
		m_pRoot->LoadChildren(*this, a_pData);
	}

	if (HASFLAG(opcode & 4) && m_pRoot->LoadNamedFrames(*this, a_pData) && HASFLAG(opcode & 2))
	{
		m_pRoot->LoadTimelines(*this, a_pData);
	}

	TASSERT(pValidateEnd == a_pData);

	return TTRUE;
}

TBOOL TXUIResource::ReadStringSection(TUINT8* buffer, TUINT32 size)
{
	TASSERT(TNULL == m_asStringTable, "StringTable must not be initialized");
	XURReader reader(buffer);

	TUINT8* pStart  = buffer;
	TUINT8* pEnd    = buffer + size;
	m_uiStringCount = 1;

	while (reader.GetPos() < pEnd)
	{
		TUINT16 stringLength = reader.ReadUInt16();
		reader.SeekFromCur(stringLength * sizeof(TWCHAR));
		m_uiStringCount++;
	}

	reader.SetPos(pStart);

	m_asStringTable       = new (TXUI::MemoryBlock()) TWCHAR*[m_uiStringCount];
	m_asStringTable[0]    = new (TXUI::MemoryBlock()) TWCHAR[1];
	m_asStringTable[0][0] = L'\0';

	for (size_t i = 1; i < m_uiStringCount; i++)
	{
		TASSERT(buffer < pEnd, "Pointer overflow");

		TUINT16 stringLength             = reader.ReadUInt16();
		m_asStringTable[i]               = new (TXUI::MemoryBlock()) TWCHAR[stringLength + 1];
		m_asStringTable[i][stringLength] = L'\0';

		for (size_t j = 0; j < stringLength; j++)
		{
			m_asStringTable[i][j] = TWCHAR(reader.ReadUInt16());
		}
	}

	return TTRUE;
}

TBOOL TXUIResource::ReadCustSection(TUINT8* buffer, TUINT32 size)
{
	m_pCustData = new (TXUI::MemoryBlock()) TUINT8[size];
	TUtil::MemCopy(m_pCustData, buffer, size);
	m_uiCustDataSize = size;
	return TTRUE;
}

TBOOL TXUIResource::ReadQuatSection(TUINT8* buffer, TUINT32 size)
{
	XURReader reader(buffer);
	m_uiQuatCount = size / sizeof(TQuaternion);

	m_aqZRotTable = new (TXUI::MemoryBlock()) TFLOAT[m_uiQuatCount];

	for (size_t i = 0; i < m_uiQuatCount; i++)
	{
		TQuaternion quaternion = { reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat() };

		TVector3 euler;
		quaternion.GetEulerXYZ(euler);

		m_aqZRotTable[i] = -euler.z;
		TASSERT(m_aqZRotTable[i] >= -TMath::PI && m_aqZRotTable[i] <= TMath::PI);
	}

	return TTRUE;
}

TINT TXUIResource::GetStringTableSize(TUINT8* pPtr, TUINT32 size)
{
	XURReader reader(pPtr);

	TUINT8* pStart       = pPtr;
	TUINT8* pEnd         = pPtr + size;
	TINT    iStringCount = 1;

	while (reader.GetPos() < pEnd)
	{
		TUINT16 stringLength = reader.ReadUInt16();
		reader.SeekFromCur(stringLength * sizeof(TWCHAR));
		iStringCount++;
	}

	reader.SetPos(pStart);

	TINT result       = iStringCount * 4 + 8;
	TINT iStringsLeft = iStringCount - 1;

	while (iStringsLeft != 0)
	{
		TASSERT(reader.GetPos() < pEnd);

		TUINT16 stringSize = reader.ReadUInt16() * sizeof(TWCHAR);
		reader.SeekFromCur(stringSize);

		result += (stringSize + 5U & 0xfffffffc);
		iStringsLeft--;
	}

	return result;
}

void TXUIResource::PushID(const TWCHAR* a_wsID)
{
	m_oIDComparisonStack.Push(a_wsID);

	if (m_ID.Length() != 0) m_ID.Concat(_TS16("."), -1);

	m_ID.Concat(a_wsID, -1);
}

void TXUIResource::PopID()
{
	const TWCHAR* top = m_oIDComparisonStack.Top();
	m_oIDComparisonStack.Pop();

	m_ID = m_ID.Left(m_ID.Length() - TStringManager::String16Length(top));
}

TXUIStringTable& TXUIResource::LookupStringTable()
{
	if (m_uiStringCount != 0)
	{
		//TXUIStringTable::Lookup()
	}
	auto lal = TXUIStringTable();

	return lal;
	// TODO: insert return statement here
}

TXUIScene* TXUIResource::CreateScene(TUINT32 a_uiIndex)
{
	XURXUIObjectData* scene = FindScene(a_uiIndex);
	if (!scene)
	{
		scene = FindScene(a_uiIndex);
		if (!scene) return TNULL;
	}
	return CreateScene(GetString(scene->m_Index));
}

TXUIScene* TXUIResource::CreateScene(const TWCHAR* a_wcName)
{
	TIMPLEMENT();
	return TNULL;
}

XURXUIObjectData* TXUIResource::FindScene(TUINT32 a_uiIndex)
{
	if (a_uiIndex < m_pRoot->m_NumChildren)
	{
		return m_pRoot->m_Children[a_uiIndex]->m_pClass->IsA(TGetClass(TXUIScene)) ? m_pRoot->m_Children[a_uiIndex] : TNULL;
	}
	return TNULL;
}

XURXUIObjectData* TXUIResource::FindScene(const TWCHAR* a_wcName)
{
	for (TINT i = 0; i < m_pRoot->m_NumChildren; i++)
	{
		if (m_pRoot->m_Children[i]->m_pClass->IsExactly(TGetClass(TXUIScene)))
		{
			if (TStringManager::String16Compare(m_asStringTable[m_pRoot->m_Children[i]->m_Index], a_wcName) == 0)
			{
				return m_pRoot->m_Children[i];
			}
		}
	}
	return TNULL;
}

XURXUIObjectData* TXUIResource::FindFirstScene(TUINT32 a_uiIndex)
{
	for (TUINT8 i = 0; i < m_pRoot->m_NumChildren; i++)
	{
		if (m_pRoot->m_Children[a_uiIndex]->m_pClass->IsA(TGetClass(TXUIScene)))
		{
			return m_pRoot->m_Children[a_uiIndex];
		}
	}
	return TNULL;
}

XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, TUINT16 a_uiType)
{
	if (a_uiType == 0) return TNULL;
	return CreateObjectData(a_rResource, a_rResource.GetString(a_uiType));
}

XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, const TWCHAR* objectName)
{
	if (TStringManager::String16Compare(objectName, _TS16("XuiCanvas"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUICanvasData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiScene"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUISceneData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiTabScene"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUITabSceneData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiFigure"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIFigureData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiVisual"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIVisualData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiImage"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIImageData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiText"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUITextData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiButton"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIButtonData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiGroup"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIGroupData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiImagePresenter"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIImagePresenterData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiTextPresenter"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUITextPresenterData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiNineGrid"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUINineGridData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiSoundXAudio"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiSoundXAct"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiLabel"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUILabelData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiEdit"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiMessageBoxButton"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiNavButton"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUINavButtonData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiBackButton"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIBackButtonData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiRadioButton"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiCheckbox"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUICheckBoxData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiProgressbar"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiSlider"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUISliderBarData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiList"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIListData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiCommonList"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUICommonListData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiRadioGroup"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiScrollEnd"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIScrollEndData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiCaret"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiListItem"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIListItemData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlPresenter"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlElement"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlControl"), -1) == 0)
	{
		TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
		return TNULL;
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiControl"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIControlData();
	}
	else if (TStringManager::String16Compare(objectName, _TS16("XuiVideo"), -1) == 0)
	{
		return new (TXUI::MemoryBlock()) XURXUIVideoData();
	}

	return TNULL;
}

void TXUIResource::Destroy()
{
	if (m_asStringTable)
	{
		for (size_t i = 0; i < m_uiStringCount; i++) delete m_asStringTable[i];

		delete[] m_asStringTable;
	}

	if (m_avVectors) delete[] m_avVectors;

	if (m_aqZRotTable) delete[] m_aqZRotTable;

	if (m_pRoot) delete m_pRoot;
}

TOSHI_NAMESPACE_END
