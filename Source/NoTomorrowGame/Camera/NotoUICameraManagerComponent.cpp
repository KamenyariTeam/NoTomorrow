// © 2025 Kamenyari. All rights reserved.

#include "NotoUICameraManagerComponent.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "NotoPlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoUICameraManagerComponent)

UNotoUICameraManagerComponent* UNotoUICameraManagerComponent::GetComponent(APlayerController* PC)
{
	if (PC != nullptr)
	{
		if (ANotoPlayerCameraManager* PCCamera = Cast<ANotoPlayerCameraManager>(PC->PlayerCameraManager))
		{
			return PCCamera->GetUICameraComponent();
		}
	}
	return nullptr;
}

UNotoUICameraManagerComponent::UNotoUICameraManagerComponent()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register a debug info hook (if not running on a dedicated server)
		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
		}
	}
}

void UNotoUICameraManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UNotoUICameraManagerComponent::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
	// Guard to ensure the view target update flag is true during the update.
	TGuardValue<bool> UpdatingGuard(bUpdatingViewTarget, true);
	ViewTarget = InViewTarget;

	// Tell the owning player camera manager to apply this view target.
	CastChecked<ANotoPlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

bool UNotoUICameraManagerComponent::NeedsToUpdateViewTarget() const
{
	// Return true if you need to update the view target each frame.
	// For now, we return false.
	return false;
}

void UNotoUICameraManagerComponent::UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime)
{
	// This function can be implemented to update OutVT with a new view target if necessary.
}

void UNotoUICameraManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// This function can be used to display debug information about the UI camera component.
	// Currently, it does nothing.
}
