// © 2025 Kamenyari. All rights reserved.

#include "NotoCameraMode_TopDownCamera.h"
#include "Curves/CurveFloat.h"

UNotoCameraMode_TopDownCamera::UNotoCameraMode_TopDownCamera()
{
	// Set default values for the viewing area
	AreaWidth = 1000.0f;
	AreaHeight = 1000.0f;
	// Default rotation that looks straight down
	DefaultPivotRotation = FRotator(-90.f, 0.f, 0.f);
}

void UNotoCameraMode_TopDownCamera::UpdateView(float DeltaTime)
{
	// Create a bounding box based on the defined area dimensions.
	FBox AreaBounds(FVector(-AreaWidth, -AreaHeight, 0.0f), FVector(AreaWidth, AreaHeight, 100.0f));

	// Get the maximum size component of the bounding box.
	const float BoundsMaxComponent = AreaBounds.GetSize().GetMax();

	// Evaluate the curve to determine the camera's distance based on the area size.
	const float CameraDistance = BoundsSizeToDistance.GetRichCurveConst()->Eval(BoundsMaxComponent);

	// Calculate the pivot location by offsetting the center of the bounds in the direction opposite to the default pivot rotation.
	FVector PivotLocation = AreaBounds.GetCenter() - DefaultPivotRotation.Vector() * CameraDistance;

	// Use the default pivot rotation as the camera rotation.
	FRotator PivotRotation = DefaultPivotRotation;

	// Update the view parameters.
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = PivotRotation;
	View.FieldOfView = FieldOfView;
}
