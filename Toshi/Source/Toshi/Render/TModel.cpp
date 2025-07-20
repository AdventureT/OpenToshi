#include "ToshiPCH.h"
#include "TModel.h"
#include "TModelManager.h"
#include "TModelInstance.h"

#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/TRender_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/TRender_SDL.h"
#endif // TOSHI_RENDERER_OPENGL

TOSHI_NAMESPACE_START

TModel::TModel()
{
	m_Flags          = Flags::None;
	m_iNumInstances  = 0;
	m_iLODCount      = 0;
	m_fUnknown       = -1.0f;
	m_pSkeleton      = TNULL;
	m_pCollisionData = TNULL;
	m_pTRB           = TNULL;
	m_bFreeOnUnload  = TTRUE;
	m_pName          = "unknown";
	TUtil::MemClear(m_LODLevels, sizeof(m_LODLevels));
	m_pDataHeader = TNULL;
	m_fUnk2       = 5.0f;
	m_fUnk3       = 20.0f;
	m_fUnk4       = 40.0f;
	m_fUnk5       = 80.0f;
}

TModel::~TModel()
{
	TASSERT(TNULL == m_pTRB);
	auto pResourceManager = T2ResourceManager::GetSingleton();

	if (GetResourceId() != T2ResourcePtr::IDINVALID)
	{
		auto pModelManager = TModelManager::GetSingleton();
		pModelManager->RemoveModel(GetResourceId());
	}

	DestroyResource();
}

void TModel::Create(const TCHAR* name, TBOOL bLoadImmediately)
{
	TASSERT(TFALSE == IsCreated());
	m_Flags.Set(Flags::Created);

	TASSERT(TFALSE == bLoadImmediately);

	CreateResource(name);
	m_pName = name;
}

void TModel::Delete()
{
	Unload();
	delete this;
}

void TModel::Unload()
{
	TIMPLEMENT();
}

TBOOL TModel::LoadTRB()
{
	TTMDBase::SkeletonHeader* pSkeletonHeader = m_pTRB->CastSymbol<TTMDBase::SkeletonHeader>(TranslateSymbolName("SkeletonHeader"));
	TSkeleton*                pSkeleton       = m_pTRB->CastSymbol<TSkeleton>(TranslateSymbolName("Skeleton"));
	TModelCollision*          pModelCollision = m_pTRB->CastSymbol<TModelCollision>(TranslateSymbolName("Collision"));

	CreateSkeleton(pSkeletonHeader, pSkeleton, TTRUE);
	CreateCollision(pModelCollision);

	TASSERT(TNULL != sm_pTRBLoadCallback);

	TBOOL bLoadRes = sm_pTRBLoadCallback(*m_pTRB, this);
	TASSERT(bLoadRes, "Failed To Load Model TRB");

	m_Flags.Set(Flags::TrbLoaded);
	return TTRUE;
}

TBOOL TModel::LoadTRB(TTRB* pTRB, TBOOL bFreeOnUnload)
{
	SetTRB(pTRB, bFreeOnUnload);
	return LoadTRB();
}

TBOOL TModel::LoadTRBFile(const TCHAR* filepath)
{
	if (m_bFreeOnUnload && m_pTRB != TNULL)
	{
		m_pTRB->Close();
		delete m_pTRB;
	}

	m_pTRB             = new TTRB();
	TTRB::ERROR iError = m_pTRB->Load(filepath);

	if (iError == TTRB::ERROR_OK)
	{
		return LoadTRB();
	}

	return TFALSE;
}

TBOOL TModel::LoadTRBFile(TFile* pFile)
{
	if (m_bFreeOnUnload && m_pTRB != TNULL)
	{
		m_pTRB->Close();
		delete m_pTRB;
	}

	m_pTRB             = new TTRB();
	TTRB::ERROR iError = m_pTRB->Load(pFile);

	if (iError == TTRB::ERROR_OK)
	{
		return LoadTRB();
	}

	return TFALSE;
}

void TModel::UnloadTRB(TBOOL bFreeTrb)
{
	if (m_pTRB != TNULL)
	{
		if (bFreeTrb && m_bFreeOnUnload)
		{
			m_pTRB->Close();
			delete m_pTRB;
		}

		m_pTRB = TNULL;
	}

	m_Flags.Unset(Flags::TrbLoaded);
	m_pCollisionData = TNULL;
	m_pSkeleton      = TNULL;
}

TModelInstance* TModel::CreateInstance()
{
	TIMPLEMENT();
	auto pInstance = new TModelInstance();

	pInstance->m_pModel = this;

	if (m_pSkeleton)
	{
		pInstance->m_pSkeletonInstance = m_pSkeleton->CreateInstance(TTRUE);

		if (!pInstance->m_pSkeletonInstance)
		{
			delete pInstance;
			return TNULL;
		}
	}

	m_iNumInstances += 1;
	return pInstance;
}

void TModel::CreateResource(const TCHAR* name)
{
	T2Resource::CreateResource(name, this, ResourceCallback, this);

	auto pModelManager = TModelManager::GetSingleton();
	pModelManager->AddModel(GetResourceId());
}

void TModel::CreateSkeleton(TTMDBase::SkeletonHeader* pSkeletonHeader, TSkeleton* pSkeleton, TBOOL bLoadAnimations)
{
	if (pSkeletonHeader != TNULL)
	{
		TASSERT(TNULL == m_pSkeleton);
		m_pSkeleton = pSkeleton;

		if (bLoadAnimations && pSkeletonHeader->m_pTKLName != TNULL)
		{
			auto& keyframeLibMgr   = TRender::GetSingletonSafe()->GetKeyframeLibraryManager();
			auto  pKeyframeLibrary = keyframeLibMgr.GetLibrary(pSkeletonHeader->m_pTKLName);

			pSkeleton->GetKeyLibraryInstance().CreateEx(
			    pKeyframeLibrary, pSkeletonHeader->m_iTKeyCount, pSkeletonHeader->m_iQKeyCount, pSkeletonHeader->m_iSKeyCount, pSkeletonHeader->m_iTBaseIndex, pSkeletonHeader->m_iQBaseIndex, pSkeletonHeader->m_iSBaseIndex
			);
		}
	}
}

void TModel::CreateCollision(TModelCollision* pModelCollision)
{
	m_pCollisionData = pModelCollision;
}

const TCHAR* TModel::TranslateSymbolName(const TCHAR* symbolName)
{
	static TCHAR s_TranslatedSymbol[512];

	if (sm_SymbolNamePrefix != TNULL)
	{
		T2String8::Format(s_TranslatedSymbol, "%s_%s", sm_SymbolNamePrefix, symbolName);
		return s_TranslatedSymbol;
	}

	return symbolName;
}
}

/*
TBOOL Toshi::TModel::LoadTMD(const TCHAR* a_tmdPath)
{
	TFile* file = TFile::Create(a_tmdPath);

	if (file != TNULL)
	{
		Header header;
		file->Read(&header, sizeof(Header));

		if (header.m_magic == TMAKEFOUR("TMDL"))
		{

		}

		file->Destroy();
	}

	return TFALSE;
}

void Toshi::TModel::CreateResource(const TCHAR* name)
{
	T2Resource::CreateResource(name, this, TNULL, this);
	
}

TBOOL Toshi::TModel::LoadTRBTMD(const TCHAR*)
{
	const TCHAR* symbol = TranslateSymbolName("SkeletonHeader");
	m_pTRB->GetSymbolAddress(symbol);
	symbol = TranslateSymbolName("Skeleton");
	m_pTRB->GetSymbolAddress(symbol);
	symbol = TranslateSymbolName("Collision");
	m_pTRB->GetSymbolAddress(symbol);
	return TFALSE;
}

TBOOL Toshi::TModel::LoadTrb()
{
	return TFALSE;
}

const TCHAR* Toshi::TModel::TranslateSymbolName(const TCHAR* a_symbolName)
{
	if (sm_SymbolNamePrefix != TNULL)
	{
		T2String8::Format(s_TranslatedSymbol, sizeof(s_TranslatedSymbol), "%s_%s", sm_SymbolNamePrefix);
		a_symbolName = s_TranslatedSymbol;
	}
	return a_symbolName;

TOSHI_NAMESPACE_END
*/
