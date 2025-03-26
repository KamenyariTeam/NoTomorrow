// © 2025 Kamenyari. All rights reserved.


#include "NotoGameMode.h"

#include "GameExperienceManagerComponent.h"
#include "NotoGameplayExperience.h"
#include "NotoGameState.h"
#include "Character/NotoCharacter.h"
#include "Character/NotoPawnData.h"
#include "Character/NotoPawnExtensionComponent.h"
#include "Core/NotoHUD.h"
#include "Core/NotoPlayerController.h"
#include "Development/NotoLogChannels.h"
#include "System/NotoAssetManager.h"

ANotoGameMode::ANotoGameMode(const FObjectInitializer& Initializer) : Super(Initializer)
{
	DefaultPawnClass = ANotoCharacter::StaticClass();
	PlayerControllerClass = ANotoPlayerController::StaticClass();
	HUDClass = ANotoHUD::StaticClass();
	GameStateClass = ANotoGameState::StaticClass();
}

const UNotoPawnData* ANotoGameMode::GetPawnDataForController(const AController* InController) const
{
	// If not, fall back to the default for the current experience
	check(GameState);
	UGameExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGameExperienceManagerComponent>();
	check(ExperienceComponent);

	if (ExperienceComponent->IsGameExperienceLoaded())
	{
		const UNotoGameplayExperience* Experience = Cast<UNotoGameplayExperience>(ExperienceComponent->GetGameExperienceChecked());
		if (Experience->DefaultPawnData != nullptr)
		{
			return Experience->DefaultPawnData;
		}

		// Experience is loaded and there's still no pawn data, fall back to the default for now
		return UNotoAssetManager::Get().GetDefaultPawnData();
	}

	// Experience not loaded yet, so there is no pawn data to be had
	return nullptr;
}

void ANotoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ANotoGameMode::OnExperienceLoaded()
{
}

bool ANotoGameMode::IsExperienceLoaded() const
{
	check(GameState);
	UGameExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGameExperienceManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsGameExperienceLoaded();
}

UClass* ANotoGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UNotoPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}
	// Fallback to the base implementation if no pawn data is available.
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ANotoGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (UNotoPawnExtensionComponent* PawnExtComp = UNotoPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
			{
				if (const UNotoPawnData* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtComp->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(LogNoto, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);

			return SpawnedPawn;
		}
		else
		{
			UE_LOG(LogNoto, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogNoto, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}

void ANotoGameMode::InitGameState()
{
	Super::InitGameState();

	// Listen for the experience load to complete
	UGameExperienceManagerComponent::CallOrRegister_OnGameExperienceLoaded(this, FGameExperienceEvent::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}
