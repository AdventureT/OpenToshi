#pragma once

#include "TGenericDList.h"

namespace Toshi
{
	template <class T>
	class TGenericEmitter : TDList<T>::TNode
	{
	public:
		TGenericEmitter() = default;
		TGenericEmitter(void* owner) 
		{ 
			m_List.m_Root.m_Next = this;
			m_List.m_Root.m_Prev = this;
			Create(owner); 
		}

		void Create(void* owner) { m_Owner = owner; }
		//void Destroy() { }

	private:

		TDList<T> m_List;
		void* m_Owner;
	};

	class TGenericListener
	{
	public:
		//void Connect(TGenericEmitter* )
	};
}