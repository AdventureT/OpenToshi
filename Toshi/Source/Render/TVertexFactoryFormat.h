#pragma once

TOSHI_NAMESPACE_START

struct TVertexStreamFormat
{
	TUINT16 m_uiVertexSize = 0;
	TUINT   m_Unk          = 0;
};

struct TVertexFactoryFormat
{
	static TCONSTEXPR_VAR TUINT MAX_NUM_STREAMS = 8;

	// $deBlob: FUNCTION 005f0e50
	TVertexFactoryFormat() = default;

	TUINT               m_uiNumStreams = 0;
	TVertexStreamFormat m_aStreamFormats[ MAX_NUM_STREAMS ];

	TUINT GetNumStreams() const { return m_uiNumStreams; }

	// $deBlob: FUNCTION 005f0fc0
	void SetNumStreams( TUINT a_uiNumStreams )
	{
		TASSERT( a_uiNumStreams < MAX_NUM_STREAMS );
		m_uiNumStreams = a_uiNumStreams;
	}

	// $deBlob: FUNCTION 005f0f50
	TVertexStreamFormat* GetStreamFormat( TUINT a_uiStream )
	{
		TASSERT( a_uiStream < m_uiNumStreams );
		return &m_aStreamFormats[ a_uiStream ];
	}
};

TOSHI_NAMESPACE_END
