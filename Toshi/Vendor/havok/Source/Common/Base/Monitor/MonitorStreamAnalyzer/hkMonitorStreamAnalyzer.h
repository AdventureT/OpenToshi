/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_MONITOR_STREAM_ANALYZER_H
#define HKBASE_MONITOR_STREAM_ANALYZER_H

#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Container/PointerMap/hkPointerMap.h>
#include <Common/Base/Container/Array/hkObjectArray.h>
#include <Common/Base/System/Io/OStream/hkOStream.h>
#include <Common/Base/Types/Color/hkColor.h>

extern const hkClass hkMonitorStreamStringMapClass;
struct hkMonitorStreamStringMap
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MONITOR, hkMonitorStreamStringMap);
	HK_DECLARE_REFLECTION();

	struct StringMap
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_BASE_CLASS, hkMonitorStreamStringMap::StringMap );
		HK_DECLARE_REFLECTION();

		HK_ALIGN(hkUint64 m_id,8); // no ulong in serialization yet, and void* will be zero 
		const char* m_string;
	};

	hkArray<struct StringMap> m_map;
};


/// Information about how to read the data from the monitors captured over a
/// given frame. You need to fill this structure in per frame captured.
/// The timing monitors are setup to capture 2 floats per call. However,
/// there are 4 "slots" in the corresponding monitor capture structure here.
/// This allows you for example to alternate between i-cache and d-cache misses
/// per frame and send them to different slots in the capture.
extern const hkClass hkMonitorStreamFrameInfoClass;
struct hkMonitorStreamFrameInfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, hkMonitorStreamFrameInfo ); 
	HK_DECLARE_REFLECTION();

	// To keep the Visual Debugger backward compatible as much as possible, try not to 
	// add any members to this struct. If you do, add them at the end (in the second public section)
	// (to make the version-ing trivial) as the VDB client does not have version-ing, and serializes this struct 
	// directly.  

		/// The heading for the frame (which will be printed out in the detailed timings file)
		/// Typically you should not forget to record what timers are being used for, and what units you
		/// are measuring times in (e.g. usec).
	const char* m_heading;

		/// The slot in our monitor capture structure, where the timer 0 should be stored
		/// If this is set to -1, timer 0 will not get stored. By default this is 0.
	hkInt32 m_indexOfTimer0;

		/// The slot in our monitor capture structure, where the timer 1 should be stored
		/// If this is set to -1, timer 1 will not get stored. By default this is 1.
	hkInt32 m_indexOfTimer1;

		/// The index of the absolute time. This should be 0 if timer O is used to
		/// store the absolute time, 1 if timer 1 is used to store the absolute time
		/// and -1 if neither timer 0 or timer 1 are used to store absolute time.
		/// This value is used to draw multiple timer threads to an image.
		/// By default this is ABSOLUTE_TIME_TIMER_0.
	enum AbsoluteTimeCounter
	{
		ABSOLUTE_TIME_TIMER_0 = 0,
		ABSOLUTE_TIME_TIMER_1 = 1,
		ABSOLUTE_TIME_NOT_TIMED = -1
	};

		/// Which timer if either is used to measure absolute time
	hkEnum<AbsoluteTimeCounter,hkUint32> m_absoluteTimeCounter;

		/// This value is used to multiply the raw data collected by the monitors and written into slot 0 of the
		/// monitor capture structure.  The value should be set depending on what information the monitors are 
		/// collecting for this slot. (e.g. timings, cache misses). For example, if you wish to display timings
		/// in microseconds you would set this value to (1 / number of clock cycles per microsecond).  Using hkBase
		/// this would be 1e6f / float(hkStopwatch::getTicksPerSecond());
		/// By default this is 1.f
	hkReal m_timerFactor0;

		/// This value is used to multiply the raw data collected by the monitors and written into slot 1 of the
		/// monitor capture structure.  See comments for m_timerFactor0 for more info. 
		/// By default this is 1.f
	hkReal m_timerFactor1;


public:

		/// The thread that this frame data came from.  This must be between 0 and the number of
		/// threads passed into the analyzer on construction.
		/// Only need to set this if you are calling captureFrameDetails().
	int m_threadId;

		/// Set automatically when you call captureFrameDetails. There is no need to initialize this value.
	int m_frameStreamStart;

		/// Set automatically when you call captureFrameDetails. There is no need to initialize this value.
	int m_frameStreamEnd;

		/// Constructor. Initializes values to defaults as documented.
	hkMonitorStreamFrameInfo();
};

	/// The input structure for the drawThreadsToTga function. Each thread is drawn as
	/// a colored bar, with different colors indicating different timers.
extern const hkClass hkMonitorStreamColorTableClass;
struct hkMonitorStreamColorTable : public hkReferencedObject
{
	//+vtable(true)
public:
	HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR );
	HK_DECLARE_REFLECTION();

	hkMonitorStreamColorTable() 
	{ 
		m_defaultColor = 0xFFFFFFFF; 
	}

		/// A mapping of timer names to colors
	struct ColorPair
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, hkMonitorStreamColorTable::ColorPair );
		HK_DECLARE_REFLECTION();

		const char* m_colorName;
		hkEnum<hkColor::ExtendedColors, hkUint32> m_color;

		ColorPair( const char* name, hkColor::ExtendedColors color )
			:	m_colorName( name), 
				m_color( color ){}
	};

	hkArray< struct ColorPair > m_colorPairs;
	hkUint32 m_defaultColor;

	hkUint32 findColor( const char* color );

	void addColor( const char* name, hkColor::ExtendedColors color );

	void setupDefaultColorTable();

	hkMonitorStreamColorTable( class hkFinishLoadedObjectFlag flag ) : hkReferencedObject(flag), m_colorPairs(flag) {}
};


/// Allows you to capture several frames (generated by hkMonitorStream)
/// and convert the data stream into a stream that can be moved across the network.
/// Also this class allows you to pretty print the results to a file.
class hkMonitorStreamAnalyzer
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MONITOR, hkMonitorStreamAnalyzer);

		///////////////////////////////////////////////////////////////////////
		//
		// Constructor, monitor capture, and reset
		//
		///////////////////////////////////////////////////////////////////////

			/// Create an analyzer given a maximum memory size. This will allocate a block of data
			/// with this size, and use it to copy monitor information when captureFrameDetails is called.
		hkMonitorStreamAnalyzer( int maxMemorySize, int numThreads = 1, int numSpus = 0 );

			/// This function must be called every frame. It copies the data between monitorStreamBegin and
			/// monitorStreamEnd, and stores the frame info structure.
		hkBool captureFrameDetails( const char* monitorStreamBegin, const char* monitorStreamEnd, hkMonitorStreamFrameInfo& info );

			/// This allows you to change the number of threads the analyzer captures
			/// This resets the analyzer. It clears all the arrays, but does not deallocate any data.
		void resetNumThreads( int numThreads = 1, int numSpus = 0 );

			/// This resets the analyzer. It clears all the arrays, but does not deallocate any data.
		void reset();

			/// Get the start of the data stream.
			/// If the steam is empty, HK_NULL will be returned.
		char* getStreamBegin();		


		///////////////////////////////////////////////////////////////////////
		//
		// Utilities to write .txt statistics files
		//
		///////////////////////////////////////////////////////////////////////

		enum
		{
			REPORT_ALL = 0xFF,
			REPORT_SUMMARIES = 0x07,
			REPORT_TOTAL = 0x01,
			REPORT_PERFRAME_TIME = 0x02,
			REPORT_PERFRAME_SUMMARY = 0x04,
			REPORT_PERFRAME_DETAIL = 0x08
		};

			/// Create a user readable statistics file of the raw data.
		void writeStatistics( hkOstream& outstream, int reportLevel = REPORT_ALL );

		///////////////////////////////////////////////////////////////////////
		//
		// Utilities to draw statistics to an image
		//
		///////////////////////////////////////////////////////////////////////
		
		struct ThreadDrawInput
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, hkMonitorStreamAnalyzer::ThreadDrawInput );
				/// The frame to start drawing from (1 by default)
			hkInt32 m_frameStart;

				/// The number of frames to draw (1 by default)
			hkInt32 m_numFrames;

				/// Layout info
			hkInt32 m_heightPerThread;
				/// Layout info
			hkInt32 m_gapBetweenThreads;

			hkInt32 m_gapBetweenFrames;

				/// Number of samples to take per frame
			hkInt32 m_outputPixelWidth;  
			
				/// Set this to > 0 if you want to limit the frame duration
			hkReal m_limitFrameTime;
			hkReal m_limitStartTime;
			
			hkBool m_warnAboutMissingTimers;

			static int HK_CALL computePerThreadHeightToFit( int textureHeight, int numFrames, int numThreads, int frameGap, int threadGap );

				/// A mapping of timer names to colors
			struct hkMonitorStreamColorTable* m_colorTable;
		};

			/// Draw the current thread data as held by this analyzer to a .tga file
		void drawThreadsToTga( const ThreadDrawInput& input, hkOstream& outStream );

		///////////////////////////////////////////////////////////////////////
		//
		// Utilities to print timers to a runtime display
		//
		///////////////////////////////////////////////////////////////////////


		struct Node; 

			/// You can use this function in conjunction with mergeTreesForCombinedThreadSummary, navigateMonitors and printMonitorsToRuntimeDisplay 
			/// to display user navigatable timers on screen. You need to be able to output text to the screen, and capture
			/// cursor key input to do this. 
			/// This call will create a tree of nodes given a monitor stream start and end. It returns the root node of the tree.
			/// For multithreaded applications, you will have a monitor stream per thread. You call this on each monitor stream,
			/// and merge the results together using mergeTreesForRuntimeDisplay. You can then use navigateMonitors and 
			/// printMonitorsToRuntimeDisplay to view the combined trees.
		static Node* HK_CALL makeStatisticsTreeForSingleFrame( const char* frameStart, const char* frameEnd, const hkMonitorStreamFrameInfo& info, const char* rootNodeName = "/", hkBool reuseNodesIfPossible = true );

			/// Given two trees, this function takes all the nodes in the subTree and merges them into the main tree. The sourceTimerId
			/// describes which slot in the subTree nodes the time is stored. This corresponds with your monitor setup, but is usually 0.
			/// The destTimerId describes which slot in the main tree nodes you wish to write. There are 4 slots, so this value must be
			/// between 0 and 3. If a node exists in the subTree but not in mainTree it is created. The smoothingFactor describes
			/// how the value is smoothed as it changes over time. Values closer to 1 smooth the timers more. 0 is unsmoothed.
		static void HK_CALL mergeTreesForCombinedThreadSummary( Node* mainTree, Node* subTree, int destTimerId, int sourceTimerId = 0, hkReal smoothingFactor = .93f );


			/// Set these values from your input device. This is used as an input to navigateMonitors
		struct CursorKeys
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, hkMonitorStreamAnalyzer::CursorKeys );
				/// up
			hkBool m_upPressed;
				/// down
			hkBool m_downPressed;
				/// left
			hkBool m_leftPressed;
				/// right
			hkBool m_rightPressed;
		};

			/// This function returns the new active node, given the current active node and the cursor keys.
		static Node* HK_CALL navigateMonitors( const CursorKeys& keys, Node* activeNodeIn );


			/// The options structure for showCombinedThreadSummaryForSingleFrame
		struct CombinedThreadSummaryOptions
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, hkMonitorStreamAnalyzer::CombinedThreadSummaryOptions );
				/// Spacing info
			int m_indentationToFirstTimerValue;
				/// Spacing info
			int m_timerColumnWidth;
				/// Spacing info
			int m_tabSpacingForTimerNames;
				/// Spacing info
			int m_tabSpacingForTimerValues;

				/// If set, only nodes with user flags set will be displayed. User flags are set by the navigateMonitors function
			bool m_displayPartialTree;
				/// The current selected node - only set if m_displayPartialTree is true
			Node* m_activeNode; 
				/// Only set if m_displayPartialTree is true
			char m_rightArrowChar;
				/// Only set if m_displayPartialTree is true
			char m_downArrowChar; 

		};

			/// This function prints the monitor tree to an hkOstream. You can create an hkOstream from an array of char,
			/// then print this array to the screen, or to file. The rootNode is the root of the tree, and the active node is the
			/// current selected node, returned by navigateMonitors. The numThreads is the number of fields (from 1 to 4)
			/// to print. The unexpanded and expanded variables allow you to insert your own graphics for these markers.
		static void HK_CALL showCombinedThreadSummaryForSingleFrame( Node* node, int numThreads, int numSpus, hkOstream& os, CombinedThreadSummaryOptions& options );

	public:

		//
		// Static internal utility methods and data used for the above functions
		//
		// 2 ppu and 6 spu threads, or 2 QuadCores.
		// The higher this number the more mem is used during timer analysis 
		enum { NUM_VALUES = 8};

		/// Data to store the parsed results of a capture
		struct Node
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, Node );

			enum NodeType
			{
				NODE_TYPE_TIMER,
				NODE_TYPE_DIRECTORY,
				NODE_TYPE_SINGLE,
			};

			hkReal			m_value[NUM_VALUES];
			hkUint16		m_count[NUM_VALUES];
			hkArray<struct Node*> m_children;
			double			m_absoluteStartTime;	// this needs double precission
			Node*			m_parent;
			const char*		m_name;	
			hkUint32		m_userFlags;
			hkEnum<NodeType,hkUint32> m_type;

			Node(Node* parent, const char* name, NodeType type );
			Node() {}

			void setTimers( const hkMonitorStreamFrameInfo& frameInfo, const hkMonitorStream::TimerCommand& start, const hkMonitorStream::TimerCommand& end);

			~Node();
		};

		static int HK_CALL findMaxTimerNameIndent( hkMonitorStreamAnalyzer::Node* node, int recursionDepth, int spacing, bool displayPartialTree );


		static void HK_CALL showCombinedThreadSummaryRec( hkOstream& os, hkMonitorStreamAnalyzer::Node* node, int recursionDepth, int numThreads, int numSpus, const CombinedThreadSummaryOptions& options );


		Node* HK_CALL makeStatisticsTreeForMultipleFrames( int threadId, hkBool reuseNodesIfPossible = false);

		void checkAllThreadsCapturedSameNumFrames() const;

		static void HK_CALL writeStatisticsDetails( hkOstream& outstream, hkArray<Node*>& nodes, int numThreads, int numSpus, int reportLevel, const char* nodeIdForFrameOverview = "", bool showMultithreadedSummary = false );

		static void HK_CALL extractStringMap( const char* frameStart, const char* frameEnd, hkPointerMap<const void*, const char*>& map );

		static void HK_CALL applyStringMap( const char* frameStart, const char* frameEnd, const hkPointerMap<const void*, const char*>& map, hkBool endianSwap );

		struct SampleInfo
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MONITOR, hkMonitorStreamAnalyzer::SampleInfo );
			hkReal m_maxSampleTime;
		};

			/// called by drawThreadsToTga (and the VDB). Call hkDeallocate on the data when you are done. Always has 4 bytes per pixel.
		static void HK_CALL writeStatisticsDetailsToTexture( hkArray<Node*>& nodes, const ThreadDrawInput& input, hkObjectArray< hkArray< hkMonitorStreamFrameInfo > >& frameInfos, int*& textureData, int& height, SampleInfo* sampleInfo = HK_NULL );
			
			/// called by drawThreadsToTga (and the VDB). Calls the above and writes it in a simple TARGA format.
		
		static void HK_CALL writeStatisticsDetailsToTga( hkArray<Node*>& nodes, const ThreadDrawInput& input, hkObjectArray< hkArray< hkMonitorStreamFrameInfo > >& frameInfos, hkOstream& outStream, SampleInfo* sampleInfo = HK_NULL );

		static Node* HK_CALL reverseLookupNodeAtTgaSample( int x, int y, hkArray<Node*>& nodes, const ThreadDrawInput& input, hkObjectArray< hkArray< hkMonitorStreamFrameInfo > >& frameInfos );
		static void HK_CALL getTimerLimits( hkArray<Node*>& nodeList, const ThreadDrawInput& input, hkObjectArray< hkArray< hkMonitorStreamFrameInfo > >& frameInfos, hkReal& maxFrameTime, hkArray<hkReal>& startTimes);

	protected:

		//
		// Data for multiple frames
		//

		hkObjectArray< hkArray< hkMonitorStreamFrameInfo > > m_frameInfos;
		hkArray<char> m_data;
		int m_numThreads;
		int m_numSpus;

	
	public:

		static hkReal g_lastFrameTime;
		const char* m_nodeIdForFrameOverview;
};


#endif // HKBASE_MONITOR_STREAM_ANALYZER_H

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
