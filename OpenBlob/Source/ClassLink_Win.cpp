#include "pch.h"
#include "ClassLink_Win.h"
#include "XUI/ABlobControllerPopup.h"
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/Windows/Input/TInputInterface_Win.h>
#include "ARevolutionary/ARevolutionary.h"
#include "ARevolutionary/ARevolutionaryCombat.h"
#include "ARevolutionary/ARevolutionaryLandmark.h"
#include "ARevolutionary/ARevolutionaryRace.h"
#include "Entities/ABaseEntity.h"
#include "Entities/ABigPaintBot/ABigPaintBot.h"
#include "Entities/ABoss/ABoss.h"
#include "Entities/ABossTrigger/ABossTrigger.h"
#include "Entities/ACar/ACar.h"
#include "Entities/ACiv.h"
#include "Entities/ACivSpawner.h"
#include "Entities/ACritter.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"
#include "Entities/AGodRay.h"
#include "Entities/AIdleEntity.h"
#include "Entities/AInkJet/AInkJet.h"
#include "Entities/AInkJetSpawner/AInkJetSpawner.h"
#include "Entities/AInky_Baton/AInky_Baton.h"
#include "Entities/AInky_Baton_Elite/AInky_Baton_Elite.h"
#include "Entities/AInky_Pen/AInky_Pen.h"
#include "Entities/AJetbikeSpawner/AJetbikeSpawner.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/APickupBonus.h"
#include "Entities/APickupEnergy.h"
#include "Entities/APickupLife.h"
#include "Entities/APickupPattern.h"
#include "Entities/APickupPowerUp.h"
#include "Entities/APickupStencil.h"
#include "Entities/APropDoor.h"
#include "Entities/APropFX.h"
#include "Entities/APropGrassBomb.h"
#include "Entities/APropHazard/AContainerHazard.h"
#include "Entities/APropHazard/APropHazard.h"
#include "Entities/APropPlatform.h"
#include "Entities/APropSkyTransition.h"
#include "Entities/APropSwitch.h"
#include "Entities/APropSwitchPressure.h"
#include "Entities/APropTrigger.h"
#include "Entities/APropTriggerEnemyDeath.h"
#include "Entities/APropTriggerEnemyGroup/APropTriggerEnemyGroup.h"
#include "Entities/APropTriggerHelp.h"
#include "Entities/APropTriggerHudMessage.h"
#include "Entities/APropTriggerInstantDeath.h"
#include "Entities/APropTriggerLevelComplete.h"
#include "Entities/APropTriggerRespawn.h"
#include "Entities/APropTriggerTimeBonus.h"
#include "Entities/APropTriggerVO.h"
#include "Entities/APropTurret/APropTurret.h"
#include "Entities/APropWater.h"
#include "Entities/APropWaterHydrant.h"
#include "Entities/APropWindZone.h"
#include "Entities/ASoundAction.h"
#include "Entities/ASquad/ASquad.h"
#include "Entities/AStreetSpawner/AStreetSpawner.h"
#include "Entities/ATankSpawner/ATankSpawner.h"
#include "Entities/ATriggerCell.h"
#include "Entities/Enemies/AEnemy.h"
#include "IO/AIOEntity.h"
#include "XUI/ABlobConfirmationMenu.h"
#include "XUI/ABlobCursorTrail.h"
#include "XUI/ABlobMenu.h"
#include "XUI/ABlobSaveLoadConfirm.h"
#include "XUI/ABlobSaveLoadMessage.h"
#include "XUI/AGUIMusicScene.h"
#include "XUI/ElementTypes/ABlobBackgroundImage.h"
#include "XUI/ElementTypes/ABlobCarouselList.h"
#include "XUI/ElementTypes/ABlobTextReveal.h"
#include "XUI/FrontEnd/ABlobChooseProfileMenu.h"
#include "XUI/FrontEnd/ABlobConceptsMenu.h"
#include "XUI/FrontEnd/ABlobCreditsMenu.h"
#include "XUI/FrontEnd/ABlobCutscenes.h"
#include "XUI/FrontEnd/ABlobDebugLevelSelectMenu.h"
#include "XUI/FrontEnd/ABlobDeleteProfileMenu.h"
#include "XUI/FrontEnd/ABlobExtrasMenu.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "XUI/FrontEnd/ABlobGalleries.h"
#include "XUI/FrontEnd/ABlobGallery.h"
#include "XUI/FrontEnd/ABlobGermanicNameEntry.h"
#include "XUI/FrontEnd/ABlobJPNNameEntryScene.h"
#include "XUI/FrontEnd/ABlobKONameSelect.h"
#include "XUI/FrontEnd/ABlobLanguageMenu.h"
#include "XUI/FrontEnd/ABlobLevelSelection.h"
#include "XUI/FrontEnd/ABlobMainMenu.h"
#include "XUI/FrontEnd/ABlobMovies.h"
#include "XUI/FrontEnd/ABlobMusicMixer.h"
#include "XUI/FrontEnd/ABlobOptionsMenu.h"
#include "XUI/FrontEnd/ABlobPopupMusicMixer.h"
#include "XUI/FrontEnd/ABlobSplash.h"
#include "XUI/FrontEnd/ABlobTrackList.h"
#include "XUI/FrontEnd/ABlobTrackSelection.h"
#include "XUI/FrontEnd/Lists/ABlobAlphabetModifiers.h"
#include "XUI/FrontEnd/Lists/ABlobDebugLevelSelect.h"
#include "XUI/FrontEnd/Lists/ABlobDebugMPMode.h"
#include "XUI/FrontEnd/Lists/ABlobDebugMenuList.h"
#include "XUI/FrontEnd/Lists/ABlobDebugNumberPlayers.h"
#include "XUI/FrontEnd/Lists/ABlobGermanicAlphabetList.h"
#include "XUI/FrontEnd/Lists/ABlobJPNAlphabetList.h"
#include "XUI/FrontEnd/Multiplayer/ABlobChoosePatternsMenu.h"
#include "XUI/FrontEnd/Multiplayer/ABlobColourSelectMenu.h"
#include "XUI/FrontEnd/Multiplayer/ABlobMPLevelSelection.h"
#include "XUI/FrontEnd/Multiplayer/ABlobMatchOptionsMenu.h"
#include "XUI/FrontEnd/Multiplayer/ABlobMatchTypeMenu.h"
#include "XUI/FrontEnd/Multiplayer/ABlobMultiplayerEngagementMenu.h"
#include "XUI/FrontEnd/Multiplayer/ABlobNumberPlayersMenu.h"
#include "XUI/InGame/ABlobChallengeDesc.h"
#include "XUI/InGame/ABlobChallengeScene.h"
#include "XUI/InGame/ABlobHelp.h"
#include "XUI/InGame/ABlobHelpFullScreen.h"
#include "XUI/InGame/ABlobInGame.h"
#include "XUI/InGame/ABlobLevelProgress.h"
#include "XUI/InGame/ABlobLoadingScreen.h"
#include "XUI/InGame/ABlobMedals.h"
#include "XUI/InGame/ABlobMultiplayerGameOver.h"
#include "XUI/InGame/ABlobMultiplayerScoreMenu.h"
#include "XUI/InGame/ABlobPauseMenu.h"
#include "XUI/InGame/ABlobRewardScreen.h"
#include "XUI/InGame/ABlobScoreMenu.h"
#include "XUI/InGame/ABlobSprintGameOver.h"
#include "XUI/InGame/ABlobSprintSummaryMenu.h"
#include "XUI/InGame/ABlobTimeupMenu.h"
#include "XUI/InGame/ABlobVistasMenu.h"
#include "XUI/InGame/HUD/ABlobAmmoHud.h"
#include "XUI/InGame/HUD/ABlobChallengeDifficultyHud.h"
#include "XUI/InGame/HUD/ABlobChallengeEndHud.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"
#include "XUI/InGame/HUD/ABlobGoalHud.h"
#include "XUI/InGame/HUD/ABlobHelpHud.h"
#include "XUI/InGame/HUD/ABlobHelpScene.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobHud_2P.h"
#include "XUI/InGame/HUD/ABlobHud_3P.h"
#include "XUI/InGame/HUD/ABlobHud_4P.h"
#include "XUI/InGame/HUD/ABlobInteractionHud.h"
#include "XUI/InGame/HUD/ABlobLeaderBoardHud.h"
#include "XUI/InGame/HUD/ABlobLetterBoxHud.h"
#include "XUI/InGame/HUD/ABlobLifeHud.h"
#include "XUI/InGame/HUD/ABlobLifeLostHud.h"
#include "XUI/InGame/HUD/ABlobMedalsHud.h"
#include "XUI/InGame/HUD/ABlobMultiplayerHud.h"
#include "XUI/InGame/HUD/ABlobPlacingHud.h"
#include "XUI/InGame/HUD/ABlobScoreFloatHud.h"
#include "XUI/InGame/HUD/ABlobScoreHud.h"
#include "XUI/InGame/HUD/ABlobShakeHud.h"
#include "XUI/InGame/HUD/ABlobSizeHud.h"
#include "XUI/InGame/HUD/ABlobTimerHud.h"
#include "XUI/InGame/HUD/ABlobVisionHud.h"
#include "XUI/InGame/HUD/ABlobVistaHud.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail copies each TClass into a local and destroys the copy; this keeps a pointer to the class' name instead
#define REFERENCE_CLASS( CLASS_NAME ) static TANONYMOUSVAR( const TCHAR* ) = CLASS_NAME::GetClassStatic()->GetName()

// $deBlob: FUNCTION 00618350
void ClassLink_Win::ReferenceGenericClasses()
{
	REFERENCE_CLASS( ABaseEntity );
	REFERENCE_CLASS( APropPlatform );
	REFERENCE_CLASS( APropDoor );
	REFERENCE_CLASS( APropTrigger );
	REFERENCE_CLASS( APropTriggerEnemyGroup );
	REFERENCE_CLASS( APropTriggerVO );
	REFERENCE_CLASS( APropTriggerHudMessage );
	REFERENCE_CLASS( APropTriggerTimeBonus );
	REFERENCE_CLASS( APropTriggerHelp );
	REFERENCE_CLASS( APropTriggerLevelComplete );
	REFERENCE_CLASS( APropTriggerInstantDeath );
	REFERENCE_CLASS( APropTriggerEnemyDeath );
	REFERENCE_CLASS( APropSwitch );
	REFERENCE_CLASS( APropSwitchPressure );
	REFERENCE_CLASS( APropWater );
	REFERENCE_CLASS( APropWaterHydrant );
	REFERENCE_CLASS( APropTriggerRespawn );
	REFERENCE_CLASS( APropGrassBomb );
	REFERENCE_CLASS( APropFX );
	REFERENCE_CLASS( APropSkyTransition );
	REFERENCE_CLASS( APropWindZone );
	REFERENCE_CLASS( ASquad );
	REFERENCE_CLASS( APropTurret );
	REFERENCE_CLASS( AGodRay );
	REFERENCE_CLASS( ABoss );
	REFERENCE_CLASS( ABossTrigger );
	REFERENCE_CLASS( ATriggerCell );
	REFERENCE_CLASS( AEnemy );
	REFERENCE_CLASS( AEnemyWrapper );
	REFERENCE_CLASS( ALeechBot2 );
	REFERENCE_CLASS( ABigPaintBot );
	REFERENCE_CLASS( ACar );
	REFERENCE_CLASS( AInky_Pen );
	REFERENCE_CLASS( AInky_Baton );
	REFERENCE_CLASS( AInky_Baton_Elite );
	REFERENCE_CLASS( ACiv );
	REFERENCE_CLASS( ACritter );
	REFERENCE_CLASS( AInkJet );
	REFERENCE_CLASS( AInkJetSpawner );
	REFERENCE_CLASS( ATankSpawner );
	REFERENCE_CLASS( ARevolutionary );
	REFERENCE_CLASS( ACivSpawner );
	REFERENCE_CLASS( AStreetSpawner );
	REFERENCE_CLASS( AJetbikeSpawner );
	REFERENCE_CLASS( ARevolutionaryRace );
	REFERENCE_CLASS( ARevolutionaryCombat );
	REFERENCE_CLASS( ARevolutionaryLandmark );
	REFERENCE_CLASS( ASoundAction );
	REFERENCE_CLASS( AIOEntity );
	REFERENCE_CLASS( APickupPowerUp );
	REFERENCE_CLASS( APickupPattern );
	REFERENCE_CLASS( APickupBonus );
	REFERENCE_CLASS( APickupStencil );
	REFERENCE_CLASS( APickupLife );
	REFERENCE_CLASS( APickupEnergy );
	REFERENCE_CLASS( APropHazard );
	REFERENCE_CLASS( AContainerHazard );
	REFERENCE_CLASS( AIdleEntity );
}

// $deBlob: FUNCTION 00618e20
void ClassLink_Win::ReferenceXUIClasses()
{
	REFERENCE_CLASS( ABlobTextReveal );
	REFERENCE_CLASS( ABlobBackgroundImage );
	REFERENCE_CLASS( ABlobCursorTrail );
	REFERENCE_CLASS( ABlobConfirmationMenu );
	REFERENCE_CLASS( ABlobSaveLoadConfirm );
	REFERENCE_CLASS( ABlobSaveLoadMessage );
	REFERENCE_CLASS( ABlobFrontEnd );
	REFERENCE_CLASS( ABlobMenu );
	REFERENCE_CLASS( ABlobSplash );
	REFERENCE_CLASS( ABlobMainMenu );
	REFERENCE_CLASS( ABlobMovies );
	REFERENCE_CLASS( ABlobGalleries );
	REFERENCE_CLASS( ABlobGallery );
	REFERENCE_CLASS( ABlobCutscenes );
	REFERENCE_CLASS( ABlobLevelSelection );
	REFERENCE_CLASS( ABlobConceptsMenu );
	REFERENCE_CLASS( ABlobDebugLevelSelectMenu );
	REFERENCE_CLASS( ABlobExtrasMenu );
	REFERENCE_CLASS( ABlobGermanicNameEntry );
	REFERENCE_CLASS( ABlobChooseProfileMenu );
	REFERENCE_CLASS( ABlobDeleteProfileMenu );
	REFERENCE_CLASS( ABlobOptionsMenu );
	REFERENCE_CLASS( ABlobLanguageMenu );
	REFERENCE_CLASS( ABlobCreditsMenu );
	REFERENCE_CLASS( ABlobTrackList );
	REFERENCE_CLASS( AGUIMusicScene );
	REFERENCE_CLASS( ABlobCarouselList );
	REFERENCE_CLASS( ABlobMusicMixer );
	REFERENCE_CLASS( ABlobTrackSelection );
	REFERENCE_CLASS( ABlobPopupMusicMixer );
	REFERENCE_CLASS( ABlobMedals );
	REFERENCE_CLASS( ABlobJPNNameEntryScene );
	REFERENCE_CLASS( ABlobJPNAlphabetList );
	REFERENCE_CLASS( ABlobKONameSelect );
	REFERENCE_CLASS( ABlobMultiplayerEngagementMenu );
	REFERENCE_CLASS( ABlobColourSelectMenu );
	REFERENCE_CLASS( ABlobNumberPlayersMenu );
	REFERENCE_CLASS( ABlobMatchTypeMenu );
	REFERENCE_CLASS( ABlobMatchOptionsMenu );
	REFERENCE_CLASS( ABlobChoosePatternsMenu );
	REFERENCE_CLASS( ABlobMPLevelSelection );
	REFERENCE_CLASS( ABlobGermanicAlphabetList );
	REFERENCE_CLASS( ABlobAlphabetModifiers );
	REFERENCE_CLASS( ABlobDebugLevelSelect );
	REFERENCE_CLASS( ABlobDebugNumberPlayers );
	REFERENCE_CLASS( ABlobDebugMPMode );
	REFERENCE_CLASS( ABlobDebugMenuList );
	REFERENCE_CLASS( ABlobInGame );
	REFERENCE_CLASS( ABlobLoadingScreen );
	REFERENCE_CLASS( ABlobPauseMenu );
	REFERENCE_CLASS( ABlobHelp );
	REFERENCE_CLASS( ABlobLevelProgress );
	REFERENCE_CLASS( ABlobScoreMenu );
	REFERENCE_CLASS( ABlobSprintSummaryMenu );
	REFERENCE_CLASS( ABlobVistasMenu );
	REFERENCE_CLASS( ABlobMultiplayerScoreMenu );
	REFERENCE_CLASS( ABlobMultiplayerGameOver );
	REFERENCE_CLASS( ABlobSprintGameOver );
	REFERENCE_CLASS( ABlobTimeupMenu );
	REFERENCE_CLASS( ABlobChallengeScene );
	REFERENCE_CLASS( ABlobVisionHud );
	REFERENCE_CLASS( ABlobChallengeEndHud );
	REFERENCE_CLASS( ABlobHelpHud );
	REFERENCE_CLASS( ABlobHelpScene );
	REFERENCE_CLASS( ABlobHelpFullScreen );
	REFERENCE_CLASS( ABlobRewardScreen );
	REFERENCE_CLASS( ABlobChallengeDesc );
	REFERENCE_CLASS( ABlobHud );
	REFERENCE_CLASS( ABlobHud_2P );
	REFERENCE_CLASS( ABlobHud_3P );
	REFERENCE_CLASS( ABlobHud_4P );
	REFERENCE_CLASS( ABlobMultiPlayerHud );
	REFERENCE_CLASS( ABlobScoreHud );
	REFERENCE_CLASS( ABlobScoreFloatHud );
	REFERENCE_CLASS( ABlobTimerHud );
	REFERENCE_CLASS( ABlobSizeHud );
	REFERENCE_CLASS( ABlobGoalHud );
	REFERENCE_CLASS( ABlobInteractionHud );
	REFERENCE_CLASS( ABlobChallengeStatusHud );
	REFERENCE_CLASS( ABlobChallengeDifficultyHud );
	REFERENCE_CLASS( ABlobShakeHud );
	REFERENCE_CLASS( ABlobAmmoHud );
	REFERENCE_CLASS( ABlobPlacingHud );
	REFERENCE_CLASS( ABlobLifeHud );
	REFERENCE_CLASS( ABlobLifeLostHud );
	REFERENCE_CLASS( ABlobVistaHud );
	REFERENCE_CLASS( ABlobLetterBoxHud );
	REFERENCE_CLASS( ABlobMedalsHud );
	REFERENCE_CLASS( ABlobLeaderBoardHud );
}

// $deBlob: FUNCTION 00619ea0
void ClassLink_Win::ReferenceSKUClasses()
{
	REFERENCE_CLASS( TRenderDX11 );
	REFERENCE_CLASS( ABlobControllerPopup );
	REFERENCE_CLASS( TInputDXInterface );
}
