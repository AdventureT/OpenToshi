#pragma once
#include "TModel.h"
#include "TSkeleton.h"

namespace Toshi {

	class TModelInstance
	{
	public:
		friend TModel;

	public:
		TModel* GetModel()
		{
			return m_pModel;
		}

		TSkeletonInstance* GetSkeletonInstance()
		{
			return m_pSkeletonInstance;
		}

	private:
		TModel* m_pModel;
		TSkeletonInstance* m_pSkeletonInstance;
	};

}