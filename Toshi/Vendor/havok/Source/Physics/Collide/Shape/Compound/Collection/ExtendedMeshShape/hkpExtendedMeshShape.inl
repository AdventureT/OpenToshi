/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

int hkpExtendedMeshShape::getNumTrianglesSubparts() const
{
	return m_numTrianglesSubparts;
}

int hkpExtendedMeshShape::getNumShapesSubparts() const
{
	return m_numShapesSubparts;
}

hkpExtendedMeshShape::TrianglesSubpart& hkpExtendedMeshShape::getTrianglesSubpartAt( int i )
{
	HK_ASSERT2(0x3031b232,  i < getNumTrianglesSubparts(), "You are trying to access a triangles-subpart which is not in the triangle subpart array");
	return m_trianglesSubparts[i];
}

const hkpExtendedMeshShape::TrianglesSubpart& hkpExtendedMeshShape::getTrianglesSubpartAt( int i ) const
{
	HK_ASSERT2(0x2bb0d984,  i < getNumTrianglesSubparts(), "You are trying to access a triangles-subpart which is not in the triangle subpart array");
	return m_trianglesSubparts[i];
}

hkpExtendedMeshShape::ShapesSubpart& hkpExtendedMeshShape::getShapesSubpartAt( int i )
{
	HK_ASSERT2(0x30312232,  i < getNumShapesSubparts(), "You are trying to access a shapes-subpart which is not in the shape subpart array");
	return m_shapesSubparts[i];
}

const hkpExtendedMeshShape::ShapesSubpart& hkpExtendedMeshShape::getShapesSubpartAt( int i ) const
{
	HK_ASSERT2(0x2bb05984,  i < getNumShapesSubparts(), "You are trying to access a shapes-subpart which is not in the shape subpart array");
	return m_shapesSubparts[i];
}

hkInt32 hkpExtendedMeshShape::getSubPartIndex( hkpShapeKey key ) const
{
	const hkUint32 subpartIndex = (key & 0x7fffffff) >> ( 32 - m_numBitsForSubpartIndex );
	return subpartIndex;
}

hkpExtendedMeshShape::SubpartType hkpExtendedMeshShape::getSubpartType( hkpShapeKey key ) const
{
	if ( !((key) & 0x80000000) )
	{
		return SUBPART_TRIANGLES;
	}
	else
	{
		return SUBPART_SHAPE;
	}
}

const hkpExtendedMeshShape::Subpart& hkpExtendedMeshShape::getSubPart( hkpShapeKey key ) const
{
	hkUint32 subpartIndex = getSubPartIndex(key);

	if ( getSubpartType(key) == SUBPART_TRIANGLES )
	{
		return m_trianglesSubparts[ subpartIndex ];
	}
	else
	{
		return m_shapesSubparts[ subpartIndex ];
	}
}

HK_FORCE_INLINE hkInt32 hkpExtendedMeshShape::getSubpartShapeKeyBase( int subpartIndex ) const
{
	if ( subpartIndex < m_numTrianglesSubparts )
	{
		return subpartIndex << ( 32 - m_numBitsForSubpartIndex );
	}

	int h = subpartIndex - m_numTrianglesSubparts;
	h =  h << ( 32 - m_numBitsForSubpartIndex );
	h |= 1<<31;
	return h;
}


hkInt32 hkpExtendedMeshShape::getTerminalIndexInSubPart( hkpShapeKey key ) const
{
	hkInt32 terminalIndex = key & ( ~0U >> m_numBitsForSubpartIndex );
	return terminalIndex;
}

hkInt32 hkpExtendedMeshShape::getNumBitsForSubpartIndex() const
{
	return m_numBitsForSubpartIndex;
}

hkReal hkpExtendedMeshShape::getRadius() const
{
	return m_triangleRadius;
}

void hkpExtendedMeshShape::setRadius(hkReal r )
{
	m_triangleRadius = r;
}

HK_FORCE_INLINE const hkVector4&	hkpExtendedMeshShape::getScaling() const
{
	return m_scaling;
}


HK_FORCE_INLINE hkpExtendedMeshShape::Subpart::Subpart(SubpartType type)
{
	m_type = type;
	// materials (default is fine)
	//m_materialIndexStridingType = MATERIAL_INDICES_INVALID;
	m_materialIndexStridingType = MATERIAL_INDICES_INT8;
	m_materialIndexStriding = 0;
	m_materialStriding = 0;
	m_numMaterials = 1;
	m_materialBase = HK_NULL;
	m_materialIndexBase = HK_NULL;
	m_materialClass = HK_NULL;
}

HK_FORCE_INLINE hkpExtendedMeshShape::TrianglesSubpart::TrianglesSubpart(): Subpart( SUBPART_TRIANGLES )
{
	m_flipAlternateTriangles = 0;
	m_extrusion.setZero4();
	m_userData = 0;

	// 'must set' values, defaults are error flags effectively for HK_ASSERTS in the cpp.
#ifdef HK_DEBUG
	m_vertexBase = HK_NULL;
	m_vertexStriding = -1;
	m_numVertices = -1;
	m_indexBase = HK_NULL;
	m_stridingType = INDICES_INVALID;
	m_indexStriding = -1;
	m_numTriangleShapes = -1;
	m_triangleOffset = -1;
#endif
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
