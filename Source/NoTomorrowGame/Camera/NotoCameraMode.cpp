// © 2025 Kamenyari. All rights reserved.

#include "NotoCameraMode.h"
#include "NotoCameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Engine/Canvas.h"

UNotoCameraMode::UNotoCameraMode()
	: FieldOfView(90.f)
	, BlendTime(0.5f)
	, BlendWeight(1.f)
{
}

UNotoCameraComponent* UNotoCameraMode::GetNotoCameraComponent() const
{
	return Cast<UNotoCameraComponent>(GetOuter());
}

UWorld* UNotoCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UNotoCameraMode::GetTargetActor() const
{
	if (UNotoCameraComponent* CamComp = GetNotoCameraComponent())
	{
		return CamComp->GetOwner();
	}
	return nullptr;
}

FVector UNotoCameraMode::GetPivotLocation() const
{
	AActor* TargetActor = GetTargetActor();
	return TargetActor ? TargetActor->GetActorLocation() + FVector(0.f, 0.f, 800.f) : FVector::ZeroVector;
}

FRotator UNotoCameraMode::GetPivotRotation() const
{
	return FRotator(-90.f, 0.f, 0.f);
}

void UNotoCameraMode::UpdateView(float DeltaTime)
{
	View.Location = GetPivotLocation();
	View.Rotation = GetPivotRotation();
	View.ControlRotation = View.Rotation; // For this example, control rotation matches the view rotation.
	View.FieldOfView = FieldOfView;
}

void UNotoCameraMode::UpdateBlending(float DeltaTime)
{
	// Blending logic can be added here if transitions are required.
	// Currently, the blend weight remains constant.
}

void UNotoCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UNotoCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.f, 1.f);
}

void UNotoCameraMode::DrawDebug(UCanvas* Canvas) const
{
	if (!Canvas)
	{
		return;
	}
	// Displays basic debug information.
	Canvas->DrawText(GEngine->GetSmallFont(), FString::Printf(TEXT("NotoCameraMode: BlendWeight=%.2f, FOV=%.1f"), BlendWeight, FieldOfView), 50.f, 50.f);
}

//////////////////////////////////////////////////////////////////////////
// UNotoCameraModeStack
//////////////////////////////////////////////////////////////////////////

UNotoCameraModeStack::UNotoCameraModeStack()
	: bIsActive(true)
{
}

void UNotoCameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;
		for (UNotoCameraMode* Mode : CameraModeStack)
		{
			if (Mode)
			{
				Mode->OnActivation();
			}
		}
	}
}

void UNotoCameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;
		for (UNotoCameraMode* Mode : CameraModeStack)
		{
			if (Mode)
			{
				Mode->OnDeactivation();
			}
		}
	}
}

void UNotoCameraModeStack::PushCameraMode(TSubclassOf<UNotoCameraMode> CameraModeClass)
{
	if (!CameraModeClass)
	{
		return;
	}

	UNotoCameraMode* NewMode = GetCameraModeInstance(CameraModeClass);
	if (!NewMode)
	{
		return;
	}

	// If already at the top, do nothing.
	if (CameraModeStack.Num() > 0 && CameraModeStack[0] == NewMode)
	{
		return;
	}

	// Remove existing instance if present.
	CameraModeStack.RemoveSingle(NewMode);

	// Insert at top and activate.
	CameraModeStack.Insert(NewMode, 0);
	NewMode->OnActivation();
}

bool UNotoCameraModeStack::EvaluateStack(float DeltaTime, FNotoCameraModeView& OutCameraModeView)
{
	if (!bIsActive || CameraModeStack.Num() == 0)
	{
		return false;
	}

	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);
	return true;
}

UNotoCameraMode* UNotoCameraModeStack::GetCameraModeInstance(TSubclassOf<UNotoCameraMode> CameraModeClass)
{
	for (UNotoCameraMode* Mode : CameraModeInstances)
	{
		if (Mode && Mode->GetClass() == CameraModeClass)
		{
			return Mode;
		}
	}
	UNotoCameraMode* NewMode = NewObject<UNotoCameraMode>(this, CameraModeClass);
	if (NewMode)
	{
		CameraModeInstances.Add(NewMode);
	}
	return NewMode;
}

void UNotoCameraModeStack::UpdateStack(float DeltaTime)
{
	for (UNotoCameraMode* Mode : CameraModeStack)
	{
		if (Mode)
		{
			Mode->UpdateCameraMode(DeltaTime);
		}
	}
}

void UNotoCameraModeStack::BlendStack(FNotoCameraModeView& OutCameraModeView) const
{
	if (CameraModeStack.Num() == 0)
	{
		return;
	}
	// For simplicity, use the top mode's view.
	OutCameraModeView = CameraModeStack[0]->GetCameraModeView();
}

void UNotoCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

	for (const UNotoCameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UNotoCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.f;
		OutTagOfTopLayer = FGameplayTag();
		return;
	}

	UNotoCameraMode* TopMode = CameraModeStack[0];
	check(TopMode);
	OutWeightOfTopLayer = TopMode->GetBlendWeight();
	OutTagOfTopLayer = TopMode->GetCameraTypeTag();
}
