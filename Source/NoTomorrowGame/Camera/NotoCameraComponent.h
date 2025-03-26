// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "NotoCameraComponent.generated.h"

class UCanvas;
class UNotoCameraMode;
class UNotoCameraModeStack;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
template <class TClass> class TSubclassOf;

// Delegate that returns a camera mode class.
DECLARE_DELEGATE_RetVal(TSubclassOf<UNotoCameraMode>, FNotoCameraModeDelegate);

/**
 * UNotoCameraComponent
 *
 * A camera component that manages camera view updates and blends camera modes.
 */
UCLASS()
class NOTOMORROWGAME_API UNotoCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UNotoCameraComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the camera component attached to an actor (if one exists).
	UFUNCTION(BlueprintPure, Category = "Noto|Camera")
	static UNotoCameraComponent* FindCameraComponent(const AActor* Actor)
	{
		return (Actor ? Actor->FindComponentByClass<UNotoCameraComponent>() : nullptr);
	}

	// Returns the actor targeted by the camera component.
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	// Delegate used to determine which camera mode should be active.
	FNotoCameraModeDelegate DetermineCameraModeDelegate;

	// Adds a temporary offset to the field of view; the offset is applied for one frame.
	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	// Draws debug information on the provided canvas.
	virtual void DrawDebug(UCanvas* Canvas) const;

	// Retrieves blend information from the camera mode stack.
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	// Called when the component is registered.
	virtual void OnRegister() override;

	// Called each frame to update the camera view.
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	// Updates the camera modes from the delegate.
	virtual void UpdateCameraModes();

protected:
	// The stack that manages and blends camera modes.
	UPROPERTY()
	TObjectPtr<UNotoCameraModeStack> CameraModeStack;

	// A one-frame offset applied to the field of view.
	float FieldOfViewOffset;
};
