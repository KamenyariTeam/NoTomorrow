// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "NotoUICameraManagerComponent.generated.h"

class ANotoPlayerCameraManager;

class AActor;
class AHUD;
class APlayerController;
class FDebugDisplayInfo;
class UCanvas;
class UObject;

/**
 * UNotoUICameraManagerComponent
 *
 * A component (attached to ANotoPlayerCameraManager) that allows for UI-related camera view management.
 */
UCLASS(Transient, Within=NotoPlayerCameraManager)
class UNotoUICameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Returns the UI camera component from the given PlayerController, if available.
	static UNotoUICameraManagerComponent* GetComponent(APlayerController* PC);

public:
	UNotoUICameraManagerComponent();	
	virtual void InitializeComponent() override;

	// Returns true if the component is currently updating the view target.
	bool IsSettingViewTarget() const { return bUpdatingViewTarget; }
	// Returns the current view target actor.
	AActor* GetViewTarget() const { return ViewTarget; }
	// Sets a new view target, optionally using a transition.
	void SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

	// Returns whether the view target needs an update.
	bool NeedsToUpdateViewTarget() const;
	// Updates the view target (if needed) during camera updates.
	void UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime);

	// Displays debug information on the HUD.
	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

private:
	// The current view target.
	UPROPERTY(Transient)
	TObjectPtr<AActor> ViewTarget;
	
	// Flag indicating that a view target update is in progress.
	UPROPERTY(Transient)
	bool bUpdatingViewTarget;
};
