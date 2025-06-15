#pragma once
#include <type_traits>

TOSHI_NAMESPACE_START

template <class T, class Node> class T2Iterator
{
public:
	static_assert(std::is_base_of<Node, T>::value, "T must be a descendant of Node");

	T2Iterator() { m_pPtr = TNULL; }

	T2Iterator(Node* pPtr) { m_pPtr = static_cast<T*>(pPtr); }

	T2Iterator(T* pPtr) { m_pPtr = pPtr; }

	/*TBOOL operator==(const T* ptr)
		{
			return m_pNode == ptr;
		}*/

	/*TBOOL operator!=(const T* ptr)
		{
			return m_pNode != ptr;
		}*/

	void operator=(const T2Iterator& other) { m_pPtr = other.m_pPtr; }

	void operator=(T* pPtr) { m_pPtr = pPtr; }

	T* operator->() const
	{
		TASSERT(m_pPtr != TNULL);
		return m_pPtr;
	}

	operator T*() const
	{
		TASSERT(m_pPtr != TNULL);
		return static_cast<T*>(m_pPtr);
	}

	T2Iterator operator++(TINT)
	{
		TASSERT(m_pPtr != TNULL);
		T2Iterator old = m_pPtr;
		m_pPtr         = static_cast<T*>(m_pPtr->Next());
		return old;
	}

	T2Iterator operator--(TINT)
	{
		TASSERT(m_pPtr != TNULL);
		T2Iterator old = m_pPtr;
		m_pPtr         = static_cast<T*>(m_pPtr->Prev());
		return old;
	}

	T2Iterator operator++()
	{
		TASSERT(m_pPtr != TNULL);
		m_pPtr = static_cast<T*>(m_pPtr->Next());
		return T2Iterator{ m_pPtr };
	}

	T2Iterator operator--()
	{
		TASSERT(m_pPtr != TNULL);
		m_pPtr = static_cast<T*>(m_pPtr->Prev());
		return T2Iterator{ m_pPtr };
	}

private:
	T* m_pPtr;
};

TOSHI_NAMESPACE_END
