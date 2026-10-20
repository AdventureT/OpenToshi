#include "pch.h"
#include "AStaticCVars.h"
#include "AConsoleState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The variables in retail's initialisation order (each marker pair: the dynamic initialiser and its atexit destructor)
// $deBlob: FUNCTION 0053bc80
// $deBlob: FUNCTION 008580b0
AConsoleVar AStaticCVars::cvCabbageTentaclePredictionHigh( "tentacle_pred_high", 0.4f );

// $deBlob: FUNCTION 0053bcb0
// $deBlob: FUNCTION 008580c0
AConsoleVar AStaticCVars::cvCabbageTentaclePredictionLow( "tentacle_pred_low", 0.1f );

// $deBlob: FUNCTION 0053bce0
// $deBlob: FUNCTION 008580d0
AConsoleVar AStaticCVars::cvCollisionBump( "collision_bump", 0.2f );

// $deBlob: FUNCTION 0053bd10
// $deBlob: FUNCTION 008580e0
AConsoleVar AStaticCVars::cvCollisionFriction( "collision_friction", 1.0f );

// $deBlob: FUNCTION 0053bd40
// $deBlob: FUNCTION 008580f0
AConsoleVar AStaticCVars::cvCollisionOffset( "collision_offset", 0.003f );

// $deBlob: FUNCTION 0053bd70
// $deBlob: FUNCTION 00858100
AConsoleVar AStaticCVars::cvRDLinesZOffset( "rd_lines_z_off", 0.0001f );

// $deBlob: FUNCTION 0053bda0
// $deBlob: FUNCTION 00858110
AConsoleVar AStaticCVars::cvDebugPickups( "pickups_debug", TFALSE );

// $deBlob: FUNCTION 0053bdc0
// $deBlob: FUNCTION 00858120
AConsoleVar AStaticCVars::cvRDTrisZOffset( "rd_tris_z_off", 0.0001f );

// $deBlob: FUNCTION 0053bdf0
// $deBlob: FUNCTION 00858130
AConsoleVar AStaticCVars::cvDisableEnemyAI( "enemy_disable_ai", TFALSE );

// $deBlob: FUNCTION 0053be10
// $deBlob: FUNCTION 00858140
AConsoleVar AStaticCVars::cvDisablePlayerAI( "player_disable_ai", TFALSE );

// $deBlob: FUNCTION 0053be30
// $deBlob: FUNCTION 00858150
AConsoleVar AStaticCVars::cvDisplayEntityIOStateOutput( "entityio_display_states", TFALSE );

// $deBlob: FUNCTION 0053be50
// $deBlob: FUNCTION 00858160
AConsoleVar AStaticCVars::cvEnableFXComposition( "fxcomposition_enable", TTRUE );

// $deBlob: FUNCTION 0053be70
// $deBlob: FUNCTION 00858170
AConsoleVar AStaticCVars::cvEnableParticles( "particles_enable", TTRUE );

// $deBlob: FUNCTION 0053be90
// $deBlob: FUNCTION 00858180
AConsoleVar AStaticCVars::cvPhysicsEnableTest( "physics_enable_test", TFALSE );

// $deBlob: FUNCTION 0053beb0
// $deBlob: FUNCTION 00858190
AConsoleVar AStaticCVars::cvEnemyTakeDamageMultiplier( "enemy_takedamagemultiplier", 1 );

// $deBlob: FUNCTION 0053bed0
// $deBlob: FUNCTION 008581a0
AConsoleVar AStaticCVars::cvEditEntityAmbientScaleMode( "edit_entity_ambient_scale_mode", TFALSE );

// $deBlob: FUNCTION 0053bef0
// $deBlob: FUNCTION 008581b0
AConsoleVar AStaticCVars::cvEntityOctTree( "entity_show_octtree", TFALSE );

// $deBlob: FUNCTION 0053bf10
// $deBlob: FUNCTION 008581c0
AConsoleVar AStaticCVars::cvEntitySelectMode( "entity_select_mode", TFALSE );

// $deBlob: FUNCTION 0053bf30
// $deBlob: FUNCTION 008581d0
AConsoleVar AStaticCVars::cvEntityShowTicks( "entity_show_ticking", TFALSE );

// $deBlob: FUNCTION 0053bf50
// $deBlob: FUNCTION 008581e0
AConsoleVar AStaticCVars::cvLockRenderFrustum( "render_lock_frustum", TFALSE );

// $deBlob: FUNCTION 0053bf70
// $deBlob: FUNCTION 008581f0
AConsoleVar AStaticCVars::cvMovePCCameraSpeed( "movepc_cameraspeed", 20.0f );

// $deBlob: FUNCTION 0053bfa0
// $deBlob: FUNCTION 00858200
AConsoleVar AStaticCVars::cvNavigatorGoal( "navigator_goal", TFALSE );

// $deBlob: FUNCTION 0053bfc0
// $deBlob: FUNCTION 00858210
AConsoleVar AStaticCVars::cvNetworkBuildSegment( "network_build_segment", -1 );

// $deBlob: FUNCTION 0053bfe0
// $deBlob: FUNCTION 00858220
AConsoleVar AStaticCVars::cvNetworkClearIsolatedNodes( "network_clear_isolated_nodes", TTRUE );

// $deBlob: FUNCTION 0053c000
// $deBlob: FUNCTION 00858230
AConsoleVar AStaticCVars::cvNetworkRender( "network_render", TFALSE );

// $deBlob: FUNCTION 0053c020
// $deBlob: FUNCTION 00858240
AConsoleVar AStaticCVars::cvNetworkShowVisitedNodes( "network_show_visited_nodes", TFALSE );

// $deBlob: FUNCTION 0053c040
// $deBlob: FUNCTION 00858250
AConsoleVar AStaticCVars::cvOptimiseLinksTolerance( "network_optimiselink_tolerance", 1.5f );

// $deBlob: FUNCTION 0053c070
// $deBlob: FUNCTION 00858260
AConsoleVar AStaticCVars::cvOptimisePath( "ai_optimise_path", TTRUE );

// $deBlob: FUNCTION 0053c090
// $deBlob: FUNCTION 00858270
AConsoleVar AStaticCVars::cvPauseGame( "pause_game", TFALSE );

// $deBlob: FUNCTION 0053c0b0
// $deBlob: FUNCTION 00858280
AConsoleVar AStaticCVars::cvPickupsAccelerationMultiplier( "pickups_accelerationmultiplier", 1.0f );

// $deBlob: FUNCTION 0053c0e0
// $deBlob: FUNCTION 00858290
AConsoleVar AStaticCVars::cvPlayerFollowMouseClick( "ai_followmouse_click", TFALSE );

// $deBlob: FUNCTION 0053c100
// $deBlob: FUNCTION 008582a0
AConsoleVar AStaticCVars::cvProjectileDebug( "projectile_debug", TFALSE );

// $deBlob: FUNCTION 0053c120
// $deBlob: FUNCTION 008582b0
AConsoleVar AStaticCVars::cvRenderInstanced( "render_instanced", TTRUE );

// $deBlob: FUNCTION 0053c140
// $deBlob: FUNCTION 008582c0
AConsoleVar AStaticCVars::cvRenderSkinned( "render_skinned", TTRUE );

// $deBlob: FUNCTION 0053c160
// $deBlob: FUNCTION 008582d0
AConsoleVar AStaticCVars::cvRenderWorld( "render_world", TTRUE );

// $deBlob: FUNCTION 0053c180
// $deBlob: FUNCTION 008582e0
AConsoleVar AStaticCVars::cvShaderLineZTest( "shader_line_ztest", TFALSE );

// $deBlob: FUNCTION 0053c1a0
// $deBlob: FUNCTION 008582f0
AConsoleVar AStaticCVars::cvShowAABBCollisions( "collision_show_aabb", TFALSE );

// $deBlob: FUNCTION 0053c1c0
// $deBlob: FUNCTION 00858300
AConsoleVar AStaticCVars::cvShowAxis( "entity_show_axis", TFALSE );

// $deBlob: FUNCTION 0053c1e0
// $deBlob: FUNCTION 00858310
AConsoleVar AStaticCVars::cvShowBBox( "entity_show_bbox", TFALSE );

// $deBlob: FUNCTION 0053c200
// $deBlob: FUNCTION 00858320
AConsoleVar AStaticCVars::cvShowCollisionObjects( "entity_show_collision_obj", TFALSE );

// $deBlob: FUNCTION 0053c220
// $deBlob: FUNCTION 00858330
AConsoleVar AStaticCVars::cvShowEntityClassName( "entity_show_classname", TFALSE );

// $deBlob: FUNCTION 0053c240
// $deBlob: FUNCTION 00858340
AConsoleVar AStaticCVars::cvShowEntityName( "entity_show_name", TFALSE );

// $deBlob: FUNCTION 0053c260
// $deBlob: FUNCTION 00858350
AConsoleVar AStaticCVars::cvShowEntityPosition( "entity_show_position", TFALSE );

// $deBlob: FUNCTION 0053c280
// $deBlob: FUNCTION 00858360
AConsoleVar AStaticCVars::cvShowGoal( "ai_show_goal", TFALSE );

// $deBlob: FUNCTION 0053c2a0
// $deBlob: FUNCTION 00858370
AConsoleVar AStaticCVars::cvPhysicsShowTests( "physics_show_tests", TFALSE );

// $deBlob: FUNCTION 0053c2c0
// $deBlob: FUNCTION 00858380
AConsoleVar AStaticCVars::cvPhysicsShowTimers( "physics_show_timers", TFALSE );

// $deBlob: FUNCTION 0053c2e0
// $deBlob: FUNCTION 00858390
AConsoleVar AStaticCVars::cvShowRenderingVisSpheres( "entity_show_rendering_vissphere", TFALSE );

// $deBlob: FUNCTION 0053c300
// $deBlob: FUNCTION 008583a0
AConsoleVar AStaticCVars::cvShowSphereCollisions( "collision_show_spere", TFALSE );

// $deBlob: FUNCTION 0053c320
// $deBlob: FUNCTION 008583b0
AConsoleVar AStaticCVars::cvSplinesRender( "splines_render", TFALSE );

// $deBlob: FUNCTION 0053c340
// $deBlob: FUNCTION 008583c0
AConsoleVar AStaticCVars::cvShowState( "entity_show_state", TFALSE );

// $deBlob: FUNCTION 0053c360
// $deBlob: FUNCTION 008583d0
AConsoleVar AStaticCVars::cvShowSteering( "ai_show_steering", TFALSE );

// $deBlob: FUNCTION 0053c380
// $deBlob: FUNCTION 008583e0
AConsoleVar AStaticCVars::cvShowSweptAABBCollisions( "collision_show_sweptaabb", TFALSE );

// $deBlob: FUNCTION 0053c3a0
// $deBlob: FUNCTION 008583f0
AConsoleVar AStaticCVars::cvShowSweptSphereCollisions( "collision_show_sweptsphere", TFALSE );

// $deBlob: FUNCTION 0053c3c0
// $deBlob: FUNCTION 00858400
AConsoleVar AStaticCVars::cvShowVisSpheres( "entity_show_vissphere", TFALSE );

// $deBlob: FUNCTION 0053c3e0
// $deBlob: FUNCTION 00858410
AConsoleVar AStaticCVars::cvShowWaypoints( "ai_show_waypoints", TFALSE );

// $deBlob: FUNCTION 0053c400
// $deBlob: FUNCTION 00858420
AConsoleVar AStaticCVars::cvSlowTime( "slow_time", 1.0f );

// $deBlob: FUNCTION 0053c430
// $deBlob: FUNCTION 00858430
AConsoleVar AStaticCVars::cvSoundMuteAllSounds( "sound_mute_all", TFALSE );

// $deBlob: FUNCTION 0053c450
// $deBlob: FUNCTION 00858440
AConsoleVar AStaticCVars::cvSoundPrintBreakpoints( "sound_print_breakpoints", TFALSE );

// $deBlob: FUNCTION 0053c470
// $deBlob: FUNCTION 00858450
AConsoleVar AStaticCVars::cvUseScratchPad( "ps2_use_scratchpad", TTRUE );

// $deBlob: FUNCTION 0053c490
// $deBlob: FUNCTION 00858460
AConsoleVar AStaticCVars::cvVOVolume( "VO_Volume", 1.0f );

// $deBlob: FUNCTION 0053c4c0
// $deBlob: FUNCTION 00858470
AConsoleVar AStaticCVars::cvVortexDebug( "vortex_debug", TFALSE );

// $deBlob: FUNCTION 005b0ec0
void AStaticCVars::RegisterStaticCVars()
{
	AConsoleState::RegisterCVar( &cvEntitySelectMode );
	AConsoleState::RegisterCVar( &cvEditEntityAmbientScaleMode );
	AConsoleState::RegisterCVar( &cvCollisionBump );
	AConsoleState::RegisterCVar( &cvCollisionFriction );
	AConsoleState::RegisterCVar( &cvCollisionOffset );
	AConsoleState::RegisterCVar( &cvNetworkBuildSegment );
	AConsoleState::RegisterCVar( &cvNetworkClearIsolatedNodes );
	AConsoleState::RegisterCVar( &cvShowEntityClassName );
	AConsoleState::RegisterCVar( &cvShowEntityName );
	AConsoleState::RegisterCVar( &cvShowEntityPosition );
	AConsoleState::RegisterCVar( &cvShowAxis );
	AConsoleState::RegisterCVar( &cvShowBBox );
	AConsoleState::RegisterCVar( &cvShowCollisionObjects );
	AConsoleState::RegisterCVar( &cvShowVisSpheres );
	AConsoleState::RegisterCVar( &cvShowRenderingVisSpheres );
	AConsoleState::RegisterCVar( &cvNetworkShowVisitedNodes );
	AConsoleState::RegisterCVar( &cvNetworkRender );
	AConsoleState::RegisterCVar( &cvSplinesRender );
	AConsoleState::RegisterCVar( &cvPlayerFollowMouseClick );
	AConsoleState::RegisterCVar( &cvShaderLineZTest );
	AConsoleState::RegisterCVar( &cvSlowTime );
	AConsoleState::RegisterCVar( &cvShowState );
	AConsoleState::RegisterCVar( &cvPauseGame );
	AConsoleState::RegisterCVar( &cvOptimiseLinksTolerance );
	AConsoleState::RegisterCVar( &cvShowWaypoints );
	AConsoleState::RegisterCVar( &cvShowSteering );
	AConsoleState::RegisterCVar( &cvOptimisePath );
	AConsoleState::RegisterCVar( &cvShowGoal );
	AConsoleState::RegisterCVar( &cvNavigatorGoal );
	AConsoleState::RegisterCVar( &cvLockRenderFrustum );
	AConsoleState::RegisterCVar( &cvRenderSkinned );
	AConsoleState::RegisterCVar( &cvRenderInstanced );
	AConsoleState::RegisterCVar( &cvRenderWorld );
	AConsoleState::RegisterCVar( &cvRDLinesZOffset );
	AConsoleState::RegisterCVar( &cvRDTrisZOffset );
	AConsoleState::RegisterCVar( &cvPhysicsShowTimers );
	AConsoleState::RegisterCVar( &cvPhysicsShowTests );
	AConsoleState::RegisterCVar( &cvPhysicsEnableTest );
	AConsoleState::RegisterCVar( &cvSoundPrintBreakpoints );
	AConsoleState::RegisterCVar( &cvUseScratchPad );
	AConsoleState::RegisterCVar( &cvDisablePlayerAI );
	AConsoleState::RegisterCVar( &cvDisableEnemyAI );
	AConsoleState::RegisterCVar( &cvVOVolume );
	AConsoleState::RegisterCVar( &cvMovePCCameraSpeed );
	AConsoleState::RegisterCVar( &cvEnemyTakeDamageMultiplier );
	AConsoleState::RegisterCVar( &cvVortexDebug );
	AConsoleState::RegisterCVar( &cvPickupsAccelerationMultiplier );
	AConsoleState::RegisterCVar( &cvDebugPickups );
	AConsoleState::RegisterCVar( &cvProjectileDebug );
	AConsoleState::RegisterCVar( &cvEntityOctTree );
	AConsoleState::RegisterCVar( &cvEntityShowTicks );
	AConsoleState::RegisterCVar( &cvShowSweptAABBCollisions );
	AConsoleState::RegisterCVar( &cvShowAABBCollisions );
	AConsoleState::RegisterCVar( &cvShowSphereCollisions );
	AConsoleState::RegisterCVar( &cvShowSweptSphereCollisions );
	AConsoleState::RegisterCVar( &cvEnableParticles );
	AConsoleState::RegisterCVar( &cvEnableFXComposition );
	AConsoleState::RegisterCVar( &cvCabbageTentaclePredictionLow );
	AConsoleState::RegisterCVar( &cvCabbageTentaclePredictionHigh );
	AConsoleState::RegisterCVar( &cvDisplayEntityIOStateOutput );
	AConsoleState::RegisterCVar( &cvSoundMuteAllSounds );
}
