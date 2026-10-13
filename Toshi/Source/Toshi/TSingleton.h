#pragma once
#include <type_traits>

TOSHI_NAMESPACE_START

#define TSINGLETON_DECLARE_ALIAS( CLASS, NAME )               \
	inline static struct SingletonHelper_##NAME               \
	{                                                         \
		CLASS* operator->() { return CLASS::GetSingleton(); } \
		operator CLASS*() { return CLASS::GetSingleton(); }   \
	} g_p##NAME

#define TSINGLETON_DECLARE_INHERITED_ALIAS( PARENT, CLASS, NAME )                    \
	inline static struct SingletonHelper_##NAME                                      \
	{                                                                                \
		CLASS* operator->() { return TSTATICCAST( CLASS, PARENT::GetSingleton() ); } \
		operator CLASS*() { return TSTATICCAST( CLASS, PARENT::GetSingleton() ); }   \
	} g_p##NAME

template <typename T>
class TSingleton
{
public:
	TSingleton( const TSingleton& )             = delete;
	TSingleton( const TSingleton&& )            = delete;
	TSingleton& operator=( const TSingleton& )  = delete;
	TSingleton& operator=( const TSingleton&& ) = delete;

	// Creates Singleton instance and returns it
	template <typename... Args>
	TFORCEINLINE static T* CreateSingleton( Args&&... args )
	{
		return new T( std::forward<Args>( args )... );
	}

	// Creates Singleton instance for a derived class and returns it
	template <typename D, typename... Args>
	TFORCEINLINE static D* CreateSingleton( Args&&... args )
	{
		TSTATICASSERT( std::is_base_of_v<T, D> == TTRUE );
		return new D( std::forward<Args>( args )... );
	}

	TFORCEINLINE static void DestroySingleton()
	{
		delete GetSingletonSafe();
		ms_pSingleton = TNULL;
	}

	// Returns pointer and asserts if it's not allocated
	TFORCEINLINE static T* GetSingletonSafe()
	{
		TASSERT( ms_pSingleton != TNULL, "Singleton instance is not created" );
		return ms_pSingleton;
	}

	TFORCEINLINE static TBOOL IsSingletonCreated()
	{
		return ms_pSingleton != TNULL;
	}

	// Returns pointer even if it's not allocated
	TFORCEINLINE static T* GetSingleton()
	{
		return ms_pSingleton;
	}

#if defined( TOSHI_MODLOADER ) || defined( TOSHI_MODLOADER_CLIENT )
	TFORCEINLINE static T* SetSingletonExplicit( T* a_pPtr )
	{
		ms_pSingleton = a_pPtr;
		return ms_pSingleton;
	}
#endif

protected:
	// de Blob: out-of-line instances (TASSERT( ms_pSingleton == TNULL ); ms_pSingleton = this), 77 classes
	// $deBlob: FUNCTION 00553e10
	// $deBlob: FUNCTION 0055d860
	// $deBlob: FUNCTION 00560bc0
	// $deBlob: FUNCTION 0056cfb0
	// $deBlob: FUNCTION 0056e780
	// $deBlob: FUNCTION 005718f0
	// $deBlob: FUNCTION 00575370
	// $deBlob: FUNCTION 0057ad00
	// $deBlob: FUNCTION 0058eaf0
	// $deBlob: FUNCTION 0059c540
	// $deBlob: FUNCTION 005a2770
	// $deBlob: FUNCTION 005a82d0
	// $deBlob: FUNCTION 005a9b10
	// $deBlob: FUNCTION 005abeb0
	// $deBlob: FUNCTION 005afc00
	// $deBlob: FUNCTION 005b32b0
	// $deBlob: FUNCTION 005b6320
	// $deBlob: FUNCTION 005b8030
	// $deBlob: FUNCTION 005c0220
	// $deBlob: FUNCTION 005c2840
	// $deBlob: FUNCTION 005c6410
	// $deBlob: FUNCTION 005d1290
	// $deBlob: FUNCTION 005d28a0
	// $deBlob: FUNCTION 005d6d70
	// $deBlob: FUNCTION 005dba20
	// $deBlob: FUNCTION 005e12c0
	// $deBlob: FUNCTION 005e2950
	// $deBlob: FUNCTION 005e5420
	// $deBlob: FUNCTION 005e70b0
	// $deBlob: FUNCTION 005e7550
	// $deBlob: FUNCTION 005e8240
	// $deBlob: FUNCTION 005e88a0
	// $deBlob: FUNCTION 005e9de0
	// $deBlob: FUNCTION 005eff90
	// $deBlob: FUNCTION 005f67e0
	// $deBlob: FUNCTION 005f8d50
	// $deBlob: FUNCTION 005f96a0
	// $deBlob: FUNCTION 005feaa0
	// $deBlob: FUNCTION 00600650
	// $deBlob: FUNCTION 00601170
	// $deBlob: FUNCTION 00602d80
	// $deBlob: FUNCTION 00603b80
	// $deBlob: FUNCTION 0060bd80
	// $deBlob: FUNCTION 0060dfa0
	// $deBlob: FUNCTION 00610ab0
	// $deBlob: FUNCTION 006137e0
	// $deBlob: FUNCTION 00614310
	// $deBlob: FUNCTION 00617650
	// $deBlob: FUNCTION 0061b970
	// $deBlob: FUNCTION 0065ef80
	// $deBlob: FUNCTION 00661990
	// $deBlob: FUNCTION 006635f0
	// $deBlob: FUNCTION 00677bf0
	// $deBlob: FUNCTION 006863b0
	// $deBlob: FUNCTION 006be390
	// $deBlob: FUNCTION 006dcc70
	// $deBlob: FUNCTION 00700790
	// $deBlob: FUNCTION 00706fc0
	// $deBlob: FUNCTION 00707010
	// $deBlob: FUNCTION 0070afa0
	// $deBlob: FUNCTION 0070b700
	// $deBlob: FUNCTION 0072d2b0
	// $deBlob: FUNCTION 0075ae00
	// $deBlob: FUNCTION 0075c4a0
	// $deBlob: FUNCTION 00763730
	// $deBlob: FUNCTION 00774010
	// $deBlob: FUNCTION 00782d40
	// $deBlob: FUNCTION 00783260
	// $deBlob: FUNCTION 0078b920
	// $deBlob: FUNCTION 0078cd00
	// $deBlob: FUNCTION 0078d2e0
	// $deBlob: FUNCTION 0078f410
	// $deBlob: FUNCTION 007904f0
	// $deBlob: FUNCTION 007913b0
	// $deBlob: FUNCTION 007cada0
	// $deBlob: FUNCTION 007dfba0
	// $deBlob: FUNCTION 007e0e00
	TFORCEINLINE TSingleton()
	{
		TASSERT( ms_pSingleton == TNULL );
		ms_pSingleton = TSTATICCAST( T, this );
	}

	// NOTE: retail keeps out-of-line instances of this destructor (mov ms_pSingleton, 0; ret), called from the derived
	// destructors and constructor unwind code. Instances in 0x560000-0x6dd6ff, in marker order:
	// ASteerPool, AKinematicsObstructionManager, AKinematicsPool, AAnimatableObjectManager, AControlComponentManager,
	// AInputHandler, AInputMapManager, AGUISystem, AInteractionComponentManager, AInteractionManager, AInteractionEventManager,
	// AQuestManager, AUnitFlagSetsManager, APlayerKnowledgeManager, AMultiplexStream, AMusicManager, ASoundManager, AGateManager,
	// AInstanceManager, ARegrowthManager, ATerrainInterface, ATreeManager, AFSMManager, AMoviePlayer, AAssetStreaming,
	// AKeyFrameLibraryManager, AInstanceLightingHelper, AGlowViewport, ARenderer, AShadowManager, ASkinLightingManager,
	// AModelRepos, AMaterialLibraryManager, APlayerManager, APlayerProfileManager, ASSeqController, AHackedQuickSave,
	// AAniEffectManager, ANPCBehaviourManager, AGUI2, AGUI2ModelRendererScreen, AParticleEffectLocatorManager,
	// AParticleEffectManager, APlayerProgress, AVehicleSkidManager, AOptions, ASimpleProjectileManager, ASubtitleManager
	// $Barnyard: FUNCTION 00572110
	// $Barnyard: FUNCTION 00573860
	// $Barnyard: FUNCTION 00574020
	// $Barnyard: FUNCTION 0057d380
	// $Barnyard: FUNCTION 0058c5c0
	// $Barnyard: FUNCTION 00595a40
	// $Barnyard: FUNCTION 00596df0
	// $Barnyard: FUNCTION 0059dea0
	// $Barnyard: FUNCTION 005a04f0
	// $Barnyard: FUNCTION 005a2150
	// $Barnyard: FUNCTION 005c5be0
	// $Barnyard: FUNCTION 005cc040
	// $Barnyard: FUNCTION 005d20b0
	// $Barnyard: FUNCTION 005d2730
	// $Barnyard: FUNCTION 005d3cc0
	// $Barnyard: FUNCTION 005d45f0
	// $Barnyard: FUNCTION 005d6330
	// $Barnyard: FUNCTION 005dd990
	// $Barnyard: FUNCTION 005dfcf0
	// $Barnyard: FUNCTION 005e26f0
	// $Barnyard: FUNCTION 005e8370
	// $Barnyard: FUNCTION 005edf30
	// $Barnyard: FUNCTION 005ffa90
	// $Barnyard: FUNCTION 006056e0
	// $Barnyard: FUNCTION 006066d0
	// $Barnyard: FUNCTION 00606be0
	// $Barnyard: FUNCTION 00607580
	// $Barnyard: FUNCTION 006085c0
	// $Barnyard: FUNCTION 0060afe0
	// $Barnyard: FUNCTION 0060cb70
	// $Barnyard: FUNCTION 0060dd60
	// $Barnyard: FUNCTION 006122f0
	// $Barnyard: FUNCTION 006158f0
	// $Barnyard: FUNCTION 0061fb30
	// $Barnyard: FUNCTION 00620110
	// $Barnyard: FUNCTION 00629cf0
	// $Barnyard: FUNCTION 0062e460
	// $Barnyard: FUNCTION 0062f9d0
	// $Barnyard: FUNCTION 00634840
	// $Barnyard: FUNCTION 00635400
	// $Barnyard: FUNCTION 006457b0
	// $Barnyard: FUNCTION 006505b0
	// $Barnyard: FUNCTION 00650c20
	// $Barnyard: FUNCTION 00651c20
	// $Barnyard: FUNCTION 00662660
	// $Barnyard: FUNCTION 00662e60
	// $Barnyard: FUNCTION 0066d160
	// $Barnyard: FUNCTION 0066e350
	// Out-of-line instantiations of the destructor (mov ms_pSingleton, 0; ret) in 0x401000-0x55ffff:
	// ALoadTimeProfiler 004024b0, ARandom 004029c0, ARootTask 00402bc0, AUnitManager 0040f600, AUnitNameManager 00411d90,
	// AHighScoreManager 0041c3b0, AHintManager 0041d280, ACheatActivator 00421c30, AGameStateController 00428b90,
	// ACameraManager 0045b950, AFadeManager 0045c580, AMiniGameManager 00469140, ACowTippingMiniGameState 004a7c20,
	// ATapperMiniGameState 004df320, ATreasureManager 00530dd0, AAnimalPopulationManager 0053a070, ABikeManager 005401c0,
	// AConversationManager 00543ed0, AGameTimeFXManager 00544650, AGameTimeManager 005462a0, AReplayManager 00547080,
	// ASimPersistData 0054b4d0, ASpeechDialogManager 0054ba20, AWeatherManager 0054c5b0, AInteriorNetworkManager 00555220
	// $Barnyard: FUNCTION 004024b0
	// $Barnyard: FUNCTION 004029c0
	// $Barnyard: FUNCTION 00402bc0
	// $Barnyard: FUNCTION 0040f600
	// $Barnyard: FUNCTION 00411d90
	// $Barnyard: FUNCTION 0041c3b0
	// $Barnyard: FUNCTION 0041d280
	// $Barnyard: FUNCTION 00421c30
	// $Barnyard: FUNCTION 00428b90
	// $Barnyard: FUNCTION 0045b950
	// $Barnyard: FUNCTION 0045c580
	// $Barnyard: FUNCTION 00469140
	// $Barnyard: FUNCTION 004a7c20
	// $Barnyard: FUNCTION 004df320
	// $Barnyard: FUNCTION 00530dd0
	// $Barnyard: FUNCTION 0053a070
	// $Barnyard: FUNCTION 005401c0
	// $Barnyard: FUNCTION 00543ed0
	// $Barnyard: FUNCTION 00544650
	// $Barnyard: FUNCTION 005462a0
	// $Barnyard: FUNCTION 00547080
	// $Barnyard: FUNCTION 0054b4d0
	// $Barnyard: FUNCTION 0054ba20
	// $Barnyard: FUNCTION 0054c5b0
	// $Barnyard: FUNCTION 00555220
	// de Blob: out-of-line instances (TASSERT( ms_pSingleton == this ); ms_pSingleton = TNULL)
	// $deBlob: FUNCTION 005a8320
	// $deBlob: FUNCTION 005a9b60
	// $deBlob: FUNCTION 005abf00
	// $deBlob: FUNCTION 005c4b70
	// $deBlob: FUNCTION 005c6490
	// $deBlob: FUNCTION 005c86a0
	// $deBlob: FUNCTION 005e76f0
	// $deBlob: FUNCTION 005e8290
	// $deBlob: FUNCTION 005e88f0
	// $deBlob: FUNCTION 005feaf0
	// $deBlob: FUNCTION 006011e0
	// $deBlob: FUNCTION 00603c00
	// $deBlob: FUNCTION 0065f1b0
	// $deBlob: FUNCTION 00661a00
	// $deBlob: FUNCTION 00663640
	// $deBlob: FUNCTION 00677cf0
	// $deBlob: FUNCTION 00686440
	// $deBlob: FUNCTION 006be410
	// $deBlob: FUNCTION 006dcda0
	// $deBlob: FUNCTION 006ffd90
	// $deBlob: FUNCTION 007007e0
	// $deBlob: FUNCTION 00707060
	// $deBlob: FUNCTION 007070a0
	// $deBlob: FUNCTION 007070f0
	// $deBlob: FUNCTION 0070aff0
	// $deBlob: FUNCTION 0070b770
	// $deBlob: FUNCTION 0072e290
	// $deBlob: FUNCTION 007740d0
	// $deBlob: FUNCTION 0078d990
	TFORCEINLINE ~TSingleton()
	{
		TASSERT( ms_pSingleton == this );
		ms_pSingleton = TNULL;
	}

	static T* ms_pSingleton;
};

template <typename T>
T* TSingleton<T>::ms_pSingleton = TNULL;

TOSHI_NAMESPACE_END
