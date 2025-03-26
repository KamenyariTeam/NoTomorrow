// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "NotoCameraMode.generated.h"

class AActor;
class UCanvas;
class UNotoCameraComponent;

/**
 * FNotoCameraModeView
 *
 * Contains the camera's location, rotation, control rotation, and field of view.
 */
struct FNotoCameraModeView
{
	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;

	FNotoCameraModeView()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, ControlRotation(FRotator::ZeroRotator)
		, FieldOfView(90.f) // Default field of view
	{
	}

	// Linearly interpolates between this view and another.
	void Blend(const FNotoCameraModeView& Other, float OtherWeight)
	{
		if (OtherWeight <= 0.f)
		{
			return;
		}
		else if (OtherWeight >= 1.f)
		{
			*this = Other;
			return;
		}

		Location = FMath::Lerp(Location, Other.Location, OtherWeight);
		Rotation = FMath::Lerp(Rotation, Other.Rotation, OtherWeight);
		ControlRotation = FMath::Lerp(ControlRotation, Other.ControlRotation, OtherWeight);
		FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
	}
};

/**
 * UNotoCameraMode
 *
 * Base class for camera modes.
 */
UCLASS(Abstract, NotBlueprintable)
class NOTOMORROWGAME_API UNotoCameraMode : public UObject
{
	GENERATED_BODY()

public:
	UNotoCameraMode();

	// Returns the camera component that owns this mode.
	UNotoCameraComponent* GetNotoCameraComponent() const;

	virtual UWorld* GetWorld() const override;

	// Returns the target actor (typically the pawn).
	virtual AActor* GetTargetActor() const;

	// Returns the computed view.
	const FNotoCameraModeView& GetCameraModeView() const { return View; }

	// Called when the mode is activated.
	virtual void OnActivation() {}

	// Called when the mode is deactivated.
	virtual void OnDeactivation() {}

	// Updates the view and blending each tick.
	void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	void SetBlendWeight(float Weight);
	
	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

	// Draws debug information.
	virtual void DrawDebug(UCanvas* Canvas) const;

protected:
	// Returns the desired camera location.
	virtual FVector GetPivotLocation() const;
	// Returns the desired camera rotation.
	virtual FRotator GetPivotRotation() const;

	// Calculates view parameters.
	virtual void UpdateView(float DeltaTime);
	// Updates blending values.
	virtual void UpdateBlending(float DeltaTime);

protected:
	// Optional tag for categorizing this mode.
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FGameplayTag CameraTypeTag;

	// The computed view.
	FNotoCameraModeView View;

	// Horizontal field of view.
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170"))
	float FieldOfView;

	// Duration for blending transitions.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// Current blend weight.
	float BlendWeight;
};

/**
 * UNotoCameraModeStack
 *
 * Manages a stack of camera modes and blends their views.
 */
UCLASS()
class NOTOMORROWGAME_API UNotoCameraModeStack : public UObject
{
	GENERATED_BODY()

public:
	UNotoCameraModeStack();

	// Activates the stack.
	bool IsStackActivate() const { return bIsActive; }
	void ActivateStack();
	// Deactivates the stack.
	void DeactivateStack();

	// Pushes a new camera mode onto the stack.
	void PushCameraMode(TSubclassOf<UNotoCameraMode> CameraModeClass);

	// Evaluates the stack and outputs the current view.
	bool EvaluateStack(float DeltaTime, FNotoCameraModeView& OutCameraModeView);

	// Draws debug information.
	void DrawDebug(UCanvas* Canvas) const;

	// Retrieves blend information from the top layer.
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	// Returns (or creates) an instance of the specified camera mode.
	UNotoCameraMode* GetCameraModeInstance(TSubclassOf<UNotoCameraMode> CameraModeClass);

	// Updates all camera modes in the stack.
	void UpdateStack(float DeltaTime);
	// Blends the camera modes to generate the final view.
	void BlendStack(FNotoCameraModeView& OutCameraModeView) const;

	bool bIsActive;

	UPROPERTY()
	TArray<UNotoCameraMode*> CameraModeInstances;

	UPROPERTY()
	TArray<UNotoCameraMode*> CameraModeStack;
};
