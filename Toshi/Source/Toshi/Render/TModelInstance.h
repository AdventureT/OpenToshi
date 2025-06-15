#pragma once
#include "TModel.h"
#include "TSkeleton.h"

TOSHI_NAMESPACE_START

class TModelInstance
{
public:
	friend TModel;

public:
	TModel* GetModel() { return m_pModel; }

	TSkeletonInstance* GetSkeletonInstance() { return m_pSkeletonInstance; }

private:
	TModel*            m_pModel;
	TSkeletonInstance* m_pSkeletonInstance;
};

TOSHI_NAMESPACE_END
