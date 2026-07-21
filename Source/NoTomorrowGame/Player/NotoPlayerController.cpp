// © 2025 Kamenyari. All rights reserved.


#include "NotoPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "CommonInputSubsystem.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerController)

ANotoPlayerController::ANotoPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> ReticleClass(TEXT("/Game/UI/Foundation/SoftwareCursors/W_GameplayReticle"));
	DefaultGameplayReticleClass = ReticleClass.Class;
}

void ANotoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	GameplayReticleClass = DefaultGameplayReticleClass;
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
	}
	ApplyCursorState();
}

void ANotoPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
	}

	if (GameplayReticleWidget)
	{
		GameplayReticleWidget->RemoveFromParent();
		GameplayReticleWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

bool ANotoPlayerController::SetPause(bool bPause, FCanUnpause CanUnpauseDelegate)
{
	const bool bPauseChanged = Super::SetPause(bPause, MoveTemp(CanUnpauseDelegate));
	if (bPauseChanged)
	{
		SetGameplayReticleVisible(!bPause);
	}
	return bPauseChanged;
}

void ANotoPlayerController::SetGameplayReticleClass(TSubclassOf<UUserWidget> InReticleClass)
{
	InReticleClass = InReticleClass ? InReticleClass : DefaultGameplayReticleClass;
	if (GameplayReticleClass == InReticleClass)
	{
		return;
	}

	GameplayReticleClass = InReticleClass;
	if (GameplayReticleWidget)
	{
		GameplayReticleWidget->RemoveFromParent();
		GameplayReticleWidget = nullptr;
	}

	if (bHasGameplayReticlePosition)
	{
		EnsureGameplayReticle();
	}
}

void ANotoPlayerController::SetGameplayReticleVisible(bool bVisible)
{
	bGameplayReticleVisible = bVisible;
	if (GameplayReticleWidget)
	{
		GameplayReticleWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	ApplyCursorState();
}

void ANotoPlayerController::SetGameplayReticlePosition(const FVector2D& ScreenPosition)
{
	GameplayReticlePosition = ScreenPosition;
	bHasGameplayReticlePosition = true;
	EnsureGameplayReticle();
}

bool ANotoPlayerController::IsUsingGamepad() const
{
	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer());
	return InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}

void ANotoPlayerController::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	ApplyCursorState();
}

void ANotoPlayerController::ApplyCursorState()
{
	if (!IsLocalController())
	{
		return;
	}

	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer());
	const bool bUsingPointer = !InputSubsystem || InputSubsystem->IsUsingPointerInput();
	SetShowMouseCursor(bUsingPointer && !bGameplayReticleVisible);
	CurrentMouseCursor = bGameplayReticleVisible ? EMouseCursor::None : EMouseCursor::Default;
}

void ANotoPlayerController::EnsureGameplayReticle()
{
	if (!GameplayReticleWidget && GameplayReticleClass)
	{
		GameplayReticleWidget = CreateWidget<UUserWidget>(this, GameplayReticleClass);
		if (GameplayReticleWidget)
		{
			GameplayReticleWidget->AddToPlayerScreen();
			GameplayReticleWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		}
	}

	if (GameplayReticleWidget)
	{
		GameplayReticleWidget->SetVisibility(bGameplayReticleVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		GameplayReticleWidget->SetPositionInViewport(GameplayReticlePosition, true);
	}
}
