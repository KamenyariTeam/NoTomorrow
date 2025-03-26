// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "NotoCameraMode.h"
#include "Curves/CurveFloat.h"
#include "NotoCameraMode_TopDownCamera.generated.h"

class UObject;

/**
 * UNotoCameraMode_TopDownCamera
 *
 * Calculates a top-down view for the camera by positioning it above a defined area.
 */
UCLASS(Abstract, Blueprintable)
class NOTOMORROWGAME_API UNotoCameraMode_TopDownCamera : public UNotoCameraMode
{
	GENERATED_BODY()

public:
	UNotoCameraMode_TopDownCamera();

protected:
	// Updates the view parameters using the defined area and pivot settings.
	virtual void UpdateView(float DeltaTime) override;

protected:
	// The half-dimensions of the area to be viewed.
	UPROPERTY(EditDefaultsOnly, Category = "Top Down")
	float AreaWidth;

	UPROPERTY(EditDefaultsOnly, Category = "Top Down")
	float AreaHeight;

	// The default rotation that determines the camera's orientation.
	UPROPERTY(EditDefaultsOnly, Category = "Top Down")
	FRotator DefaultPivotRotation;

	// Curve that maps the area size to a desired camera distance.
	UPROPERTY(EditDefaultsOnly, Category = "Top Down")
	FRuntimeFloatCurve BoundsSizeToDistance;
};
