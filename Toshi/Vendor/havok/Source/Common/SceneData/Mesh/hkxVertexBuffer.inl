/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkxVertexBuffer::hkxVertexBuffer()
	: m_vertexDataClass(HK_NULL), m_vertexDesc(HK_NULL) 
{ 
}
		
inline int hkxVertexBuffer::getNumVertices() const 
{ 
	return m_numVertexData; 
}

inline const char* hkxVertexBuffer::getVertexData() const 
{ 
	return reinterpret_cast<char*>( m_vertexData ); 
}

inline char* hkxVertexBuffer::getVertexData() 
{ 
	return reinterpret_cast<char*>( m_vertexData ); 
}

inline const hkxVertexDescription* hkxVertexBuffer::getVertexDesc() const 
{ 
	return m_vertexDesc; 
}

inline hkxVertexDescription* hkxVertexBuffer::getVertexDesc() 
{ 
	return m_vertexDesc; 
}

inline const hkClass* hkxVertexBuffer::getVertexDataClass( ) const
{
	return m_vertexDataClass;
}

inline void hkxVertexBuffer::setVertexDataClass( const hkClass* klass ) 
{ 
	m_vertexDataClass = klass; 
}

inline void hkxVertexBuffer::setVertexDataPtr( void* data, int numDataItems ) 				
{ 
	m_vertexData = data; 
	m_numVertexData = numDataItems;
}

inline void hkxVertexBuffer::setVertexCount(int c) 
{
	m_numVertexData = c; 
}

inline void hkxVertexBuffer::setVertexDesc(hkxVertexDescription* desc)
{
	m_vertexDesc = desc;

}

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
