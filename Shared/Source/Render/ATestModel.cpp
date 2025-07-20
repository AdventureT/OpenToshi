#include "pch.h"
#include "ATestModel.h"

#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Shaders/SysShader/TSysShaderHAL.h>

struct SysMeshVertex
{
	Toshi::TVector3 Position;
	uint32_t        Color;
	Toshi::TVector2 UV;
};

ATestModel::ATestModel()
{
	m_Position = { 0.0f, 0.0f, 6.0f, 1.0f };

	m_AssetTRB.Load("Data\\LEVELS\\PRODUCTION\\Singleplayer\\Abyss\\RegionAssets.trb");
	Toshi::TAssetInit::InitAssets(m_AssetTRB, TTRUE, TFALSE);

	CreateCube();
	CreatePlane();
	CreateTriangle();
}

ATestModel::~ATestModel()
{
	Toshi::TAssetInit::DeinitAssets(m_AssetTRB);

	m_pCube->DestroyResource();
	m_pPlane->DestroyResource();
	m_pTriangle->DestroyResource();
	delete m_pCubeMaterial;
	delete m_pPlaneMaterial;
	delete m_pTriangleMaterial;
}

void ATestModel::Render(float deltaTime)
{
	auto pRenderContext = Toshi::TRender::GetSingleton()->GetCurrentRenderContext();

	Toshi::TMatrix44 worldView = pRenderContext->GetWorldViewMatrix();
	Toshi::TMatrix44 modelView;

	/// Cube
	Toshi::TMatrix44 rotationMatrix;
	rotationMatrix.Identity();
	rotationMatrix.RotateY(m_CubeRotation);

	Toshi::TMatrix44 modelMatrix;
	modelMatrix.Identity();
	modelMatrix.SetTranslation(m_Position);

	modelView.Multiply(worldView, modelMatrix);
	modelView.Multiply(rotationMatrix);
	pRenderContext->SetModelViewMatrix(modelView);

	m_pCube->Render();
	m_CubeRotation += deltaTime;

	/// Plane
	modelMatrix.Identity();
	modelMatrix.Scale(10.0f, 1.0f, 10.0f);
	modelMatrix.SetTranslation(m_Position + Toshi::TVector3(0, 0.1f, 0));

	modelView.Multiply(worldView, modelMatrix);
	pRenderContext->SetModelViewMatrix(modelView);

	m_pPlane->Render();

	/// Triangle
	rotationMatrix.RotateY(0.0f);
	modelMatrix.Identity();
	modelMatrix.SetTranslation(m_Position + Toshi::TVector3(0, 0.1f, 0));
	modelMatrix.Multiply(rotationMatrix);

	modelView.Multiply(worldView, modelMatrix);
	pRenderContext->SetModelViewMatrix(modelView);

	m_pTriangle->Render();
}

void ATestModel::CreateCube()
{
	static SysMeshVertex s_Vertices[] = {
		{ {
		      -1.0f,
		      1.0f,
		      -1.0f,
		  },
		  0xFFFFFFFF,
		  { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, -1.0f }, 0xFFFFFFFF, { 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f }, 0xFFFFFFFF, { 0.0f, 1.0f } },
		{ { 1.0f, -1.0f, -1.0f }, 0xFFFFFFFF, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, 1.0f }, 0xFFFFFFFF, { 0.0f, 1.0f } },
		{ { 1.0f, -1.0f, 1.0f }, 0xFFFFFFFF, { 1.0f, 1.0f } },
		{ { -1.0f, 1.0f, 1.0f }, 0xFFFFFFFF, { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, 1.0f }, 0xFFFFFFFF, { 1.0f, 0.0f } },
	};

	static TUINT16 s_Indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 0, 1
	};

	auto pTexManager = Toshi::TTextureManager::GetSingletonSafe();
	auto pSysShader  = Toshi::TSysShader::GetSingletonSafe();

	m_pCubeMaterial = pSysShader->CreateMaterial();
	m_pCubeMaterial->SetFlag(Toshi::TMaterial::Flags_AlphaUpdate, TTRUE);
	m_pCubeMaterial->Create(Toshi::TSysMaterialHAL::BlendMode::Default);

	auto pTexture = pTexManager->FindTexture("prop_fountain_02.tga");
	pTexture->SetAlphaEnabled(TFALSE);
	m_pCubeMaterial->SetTexture(0, pTexture);

	auto pMesh = pSysShader->CreateMesh("test_cube");
	pMesh->Create(0, sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(TUINT16));

	Toshi::TSysMesh::TLockBuffer lock;

	if (pMesh->Lock(&lock))
	{
		Toshi::TUtil::MemCopy(lock.pVertexBufferData, s_Vertices, sizeof(s_Vertices));
		Toshi::TUtil::MemCopy(lock.pIndexBufferData, s_Indices, sizeof(s_Indices));
		pMesh->Unlock(sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(TUINT16));
	}

	pMesh->SetMaterial(m_pCubeMaterial);
	m_pCube = pMesh;
}

void ATestModel::CreatePlane()
{
	static SysMeshVertex s_Vertices[] = {
		{ { -1.0f, 1.0f, -1.0f }, 0xFFFFFFFF, { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, -1.0f }, 0xFFFFFFFF, { 12.0f, 0.0f } },
		{ { -1.0f, 1.0f, 1.0f }, 0xFFFFFFFF, { 0.0f, 12.0f } },
		{ { 1.0f, 1.0f, 1.0f }, 0xFFFFFFFF, { 12.0f, 12.0f } },
	};

	static TUINT16 s_Indices[] = {
		0, 1, 2, 3
	};

	auto pTexManager = Toshi::TTextureManager::GetSingletonSafe();
	auto pSysShader  = Toshi::TSysShader::GetSingletonSafe();

	m_pPlaneMaterial = pSysShader->CreateMaterial();
	m_pPlaneMaterial->SetFlag(Toshi::TMaterial::Flags_AlphaUpdate, TTRUE);
	m_pPlaneMaterial->Create(Toshi::TSysMaterialHAL::BlendMode::Default);

	auto pTexture = pTexManager->FindTexture("slum_island_colour.tga");
	pTexture->SetAlphaEnabled(TFALSE);
	pTexture->SetSamplerId(3);
	m_pPlaneMaterial->SetTexture(0, pTexture);

	auto pMesh = pSysShader->CreateMesh("test_cube");
	pMesh->Create(0, sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(TUINT16));

	Toshi::TSysMesh::TLockBuffer lock;

	if (pMesh->Lock(&lock))
	{
		Toshi::TUtil::MemCopy(lock.pVertexBufferData, s_Vertices, sizeof(s_Vertices));
		Toshi::TUtil::MemCopy(lock.pIndexBufferData, s_Indices, sizeof(s_Indices));
		pMesh->Unlock(sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(TUINT16));
	}

	pMesh->SetMaterial(m_pPlaneMaterial);
	m_pPlane = pMesh;
}

void ATestModel::CreateTriangle()
{
	static SysMeshVertex s_Vertices[] = {
		{ { -0.5f, 0.5f, 0.0f }, 0xFFFFFFFF, { 0.0f, 0.0f } },
		{ { 0.5f, 0.5f, 0.0f }, 0xFFFFFFFF, { 1.0f, 0.0f } },
		{ { 0.0f, -0.5f, 0.0f }, 0xFFFFFFFF, { 0.5f, 1.0f } },
	};

	static TUINT16 s_Indices[] = {
		0, 1, 2
	};

	auto pTexManager = Toshi::TTextureManager::GetSingletonSafe();
	auto pSysShader  = Toshi::TSysShader::GetSingletonSafe();

	m_pTriangleMaterial = pSysShader->CreateMaterial();
	m_pTriangleMaterial->SetFlag(Toshi::TMaterial::Flags_AlphaUpdate, TTRUE);
	m_pTriangleMaterial->Create(Toshi::TSysMaterialHAL::BlendMode::Default);

	auto pTexture = pTexManager->FindTexture("water.tga");
	pTexture->SetAlphaEnabled(TFALSE);
	pTexture->SetSamplerId(3);
	m_pTriangleMaterial->SetTexture(0, pTexture);

	auto pMesh = pSysShader->CreateMesh("test_triangle");
	pMesh->Create(0, sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(TUINT16));

	Toshi::TSysMesh::TLockBuffer lock;

	if (pMesh->Lock(&lock))
	{
		Toshi::TUtil::MemCopy(lock.pVertexBufferData, s_Vertices, sizeof(s_Vertices));
		Toshi::TUtil::MemCopy(lock.pIndexBufferData, s_Indices, sizeof(s_Indices));
		pMesh->Unlock(sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(TUINT16));
	}

	pMesh->SetMaterial(m_pTriangleMaterial);
	m_pTriangle = pMesh;
}
