// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "NotoPlayerCameraManager.generated.h"

class FDebugDisplayInfo;
class UCanvas;
class UObject;
class UNotoUICameraManagerComponent;

#define NOTO_CAMERA_DEFAULT_FOV          (80.0f)
#define NOTO_CAMERA_DEFAULT_PITCH_MIN    (-89.0f)
#define NOTO_CAMERA_DEFAULT_PITCH_MAX    (89.0f)

/**
 * ANotoPlayerCameraManager
 *
 * Base player camera manager class used by the project.
 */
UCLASS(NotPlaceable, MinimalAPI)
class ANotoPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ANotoPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	/** Returns the UI camera component, which controls the camera when UI should be prioritized. */
	UNotoUICameraManagerComponent* GetUICameraComponent() const;

protected:
	/** Updates the view target. If the UI camera needs to override, its view is applied. */
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	/** Displays debug information on the canvas. */
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

private:
	/** The UI Camera Component, used to control the camera when UI is active. */
	UPROPERTY(Transient)
	TObjectPtr<UNotoUICameraManagerComponent> UICamera;
};
