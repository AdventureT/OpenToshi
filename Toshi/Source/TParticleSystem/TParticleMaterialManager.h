#pragma once
#include "TParticleMaterial.h"


TOSHI_NAMESPACE_START


class TParticleMaterialManager
{
public:
	TParticleMaterialManager();

	TParticleMaterial* FindMaterial( const TCHAR* a_szName ) const;
	void               UpdateMaterials();
	void               AddMaterial( TParticleMaterial* a_pMaterial );
	void               RemoveMaterial( TParticleMaterial* a_pMaterial );

	TINT GetMaterialCount() const { return m_iNumMaterials; }

	// The material of the RENDERSTYLE_MESH particles (ID -1: rendered by the particles themselves)
	TParticleMaterial* GetMeshMaterial() { return &m_oMeshMaterial; }

private:
	TINT               m_iNumMaterials;   // 0x00
	TParticleMaterial* m_pMaterialsRoot;  // 0x04
	TParticleMaterial  m_oMeshMaterial;   // 0x08
};

TOSHI_NAMESPACE_END
