#pragma once
#include "AConsoleVar.h"

//-----------------------------------------------------------------------------
// Purpose: the game's static console variables. Retail
// creates them in dynamic initialisers and
// AAppInitState::OnUpdate registers them all with AConsoleState (RegisterStaticCVars).
// NOTE: the console state that would edit them is never created in retail (see AConsoleState.h), so they keep their
// defaults unless something sets them in code.
//-----------------------------------------------------------------------------
class AStaticCVars
{
public:
	// Registers every static console variable with AConsoleState
	static void RegisterStaticCVars();

public:
	static AConsoleVar cvCabbageTentaclePredictionHigh; // "tentacle_pred_high"
	static AConsoleVar cvCabbageTentaclePredictionLow;  // "tentacle_pred_low"
	static AConsoleVar cvCollisionBump;                 // "collision_bump"
	static AConsoleVar cvCollisionFriction;             // "collision_friction"
	static AConsoleVar cvCollisionOffset;               // "collision_offset"
	static AConsoleVar cvRDLinesZOffset;                // "rd_lines_z_off"
	static AConsoleVar cvDebugPickups;                  // "pickups_debug"
	static AConsoleVar cvRDTrisZOffset;                 // "rd_tris_z_off"
	static AConsoleVar cvDisableEnemyAI;                // "enemy_disable_ai"
	static AConsoleVar cvDisablePlayerAI;               // "player_disable_ai"
	static AConsoleVar cvDisplayEntityIOStateOutput;    // "entityio_display_states"
	static AConsoleVar cvEnableFXComposition;           // "fxcomposition_enable"
	static AConsoleVar cvEnableParticles;               // "particles_enable"
	static AConsoleVar cvPhysicsEnableTest;             // "physics_enable_test"
	static AConsoleVar cvEnemyTakeDamageMultiplier;     // "enemy_takedamagemultiplier"
	static AConsoleVar cvEditEntityAmbientScaleMode;    // "edit_entity_ambient_scale_mode"
	static AConsoleVar cvEntityOctTree;                 // "entity_show_octtree"
	static AConsoleVar cvEntitySelectMode;              // "entity_select_mode"
	static AConsoleVar cvEntityShowTicks;               // "entity_show_ticking"
	static AConsoleVar cvLockRenderFrustum;             // "render_lock_frustum"
	static AConsoleVar cvMovePCCameraSpeed;             // "movepc_cameraspeed"
	static AConsoleVar cvNavigatorGoal;                 // "navigator_goal"
	static AConsoleVar cvNetworkBuildSegment;           // "network_build_segment"
	static AConsoleVar cvNetworkClearIsolatedNodes;     // "network_clear_isolated_nodes"
	static AConsoleVar cvNetworkRender;                 // "network_render"
	static AConsoleVar cvNetworkShowVisitedNodes;       // "network_show_visited_nodes"
	static AConsoleVar cvOptimiseLinksTolerance;        // "network_optimiselink_tolerance"
	static AConsoleVar cvOptimisePath;                  // "ai_optimise_path"
	static AConsoleVar cvPauseGame;                     // "pause_game"
	static AConsoleVar cvPickupsAccelerationMultiplier; // "pickups_accelerationmultiplier"
	static AConsoleVar cvPlayerFollowMouseClick;        // "ai_followmouse_click"
	static AConsoleVar cvProjectileDebug;               // "projectile_debug"
	static AConsoleVar cvRenderInstanced;               // "render_instanced"
	static AConsoleVar cvRenderSkinned;                 // "render_skinned"
	static AConsoleVar cvRenderWorld;                   // "render_world"
	static AConsoleVar cvShaderLineZTest;               // "shader_line_ztest"
	static AConsoleVar cvShowAABBCollisions;            // "collision_show_aabb"
	static AConsoleVar cvShowAxis;                      // "entity_show_axis"
	static AConsoleVar cvShowBBox;                      // "entity_show_bbox"
	static AConsoleVar cvShowCollisionObjects;          // "entity_show_collision_obj"
	static AConsoleVar cvShowEntityClassName;           // "entity_show_classname"
	static AConsoleVar cvShowEntityName;                // "entity_show_name"
	static AConsoleVar cvShowEntityPosition;            // "entity_show_position"
	static AConsoleVar cvShowGoal;                      // "ai_show_goal"
	static AConsoleVar cvPhysicsShowTests;              // "physics_show_tests"
	static AConsoleVar cvPhysicsShowTimers;             // "physics_show_timers"
	static AConsoleVar cvShowRenderingVisSpheres;       // "entity_show_rendering_vissphere"
	static AConsoleVar cvShowSphereCollisions;          // "collision_show_spere"
	static AConsoleVar cvSplinesRender;                 // "splines_render"
	static AConsoleVar cvShowState;                     // "entity_show_state"
	static AConsoleVar cvShowSteering;                  // "ai_show_steering"
	static AConsoleVar cvShowSweptAABBCollisions;       // "collision_show_sweptaabb"
	static AConsoleVar cvShowSweptSphereCollisions;     // "collision_show_sweptsphere"
	static AConsoleVar cvShowVisSpheres;                // "entity_show_vissphere"
	static AConsoleVar cvShowWaypoints;                 // "ai_show_waypoints"
	static AConsoleVar cvSlowTime;                      // "slow_time"
	static AConsoleVar cvSoundMuteAllSounds;            // "sound_mute_all"
	static AConsoleVar cvSoundPrintBreakpoints;         // "sound_print_breakpoints"
	static AConsoleVar cvUseScratchPad;                 // "ps2_use_scratchpad"
	static AConsoleVar cvVOVolume;                      // "VO_Volume"
	static AConsoleVar cvVortexDebug;                   // "vortex_debug"
};
