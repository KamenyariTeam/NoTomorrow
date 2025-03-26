// © 2025 Kamenyari. All rights reserved.

#include "NotoPlayerCameraManager.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "NotoCameraComponent.h"
#include "NotoUICameraManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerCameraManager)

ANotoPlayerCameraManager::ANotoPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFOV = NOTO_CAMERA_DEFAULT_FOV;
	ViewPitchMin = NOTO_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = NOTO_CAMERA_DEFAULT_PITCH_MAX;

	UICamera = CreateDefaultSubobject<UNotoUICameraManagerComponent>(TEXT("UICamera"));
}

UNotoUICameraManagerComponent* ANotoPlayerCameraManager::GetUICameraComponent() const
{
	return UICamera;
}

void ANotoPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// If the UI camera is active, let it update the view target.
	if (UICamera && UICamera->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICamera->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void ANotoPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("NotoPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);
	if (const UNotoCameraComponent* CameraComponent = UNotoCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}
