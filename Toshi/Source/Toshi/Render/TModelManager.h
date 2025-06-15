#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi2/T2ModelPtr.h"
#include "TModel.h"

TOSHI_NAMESPACE_START

class TModelManager : public TSingleton<TModelManager>
{
public:
	TModelManager() {}

	void AddModel(T2ModelPtr model);
	void RemoveModel(T2ModelPtr model);
	void FindModel(T2ModelPtr& outModel, const TCHAR* name);

	T2ModelPtr GetFirstModel() const { return m_HeadModel; }

	T2ModelPtr GetNextModel(T2ModelPtr model) const { return model->m_NextModelResource; }

private:
	T2ModelPtr m_HeadModel;
};

TOSHI_NAMESPACE_END
