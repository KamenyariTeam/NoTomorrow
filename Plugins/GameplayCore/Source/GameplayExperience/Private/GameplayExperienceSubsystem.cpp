#include "GameplayExperienceSubsystem.h"

#include "GameExperienceManagerComponent.h"
#include "GameplayExperience.h"
#include "GameplayExperienceProvider.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

UGameExperienceSettings::UGameExperienceSettings()
{
	DefaultExperienceType = UGameplayExperience::StaticClass();

	CategoryName = TEXT("Game");
}

UGameplayExperienceSubsystem* UGameplayExperienceSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	return World->GetGameInstance()->GetSubsystem<UGameplayExperienceSubsystem>();
}

bool UGameplayExperienceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// allow creating derived classes of this subsystem
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(GetClass(), DerivedClasses, true);

	for (UClass* SubsystemClass: DerivedClasses)
	{
		const USubsystem* ChildSubsystem = SubsystemClass->GetDefaultObject<USubsystem>();
		if (ChildSubsystem->ShouldCreateSubsystem(Outer))
		{
			return false;
		}
	}
	
	return Super::ShouldCreateSubsystem(Outer);
}

void UGameplayExperienceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &ThisClass::HandleInitGame);
}

void UGameplayExperienceSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);
	
	Super::Deinitialize();
}

void UGameplayExperienceSubsystem::LoadGameExperience(FPrimaryAssetId ExperienceId)
{
	if (!ExperienceId.IsValid())
	{
		UE_LOG(LogGameExperience, Error, TEXT("%s: failed to identify experience %s"), *FString(__FUNCTION__), *ExperienceId.ToString());
		return;
	}
	
	const UWorld* World = GetGameInstance()->GetWorld();
	check(World);

	const AGameStateBase* GameState = World->GetGameState();
	check(GameState);

	UGameExperienceManagerComponent* ExperienceComponent = World->GetGameState()->FindComponentByClass<UGameExperienceManagerComponent>();
	if (!IsValid(ExperienceComponent))
	{
		UE_LOG(LogGameExperience, Error, TEXT("%s: unable to find UGameExperienceManagerComponent for game state %s"),
			*FString(__FUNCTION__), *GetNameSafe(GameState));
		return;
	}

	ExperienceComponent->LoadGameExperience(ExperienceId);
}

void UGameplayExperienceSubsystem::HandleInitGame(const FActorsInitializedParams& Params)
{
	UWorld* World = Params.World;
	if (!World->IsGameWorld())
	{
		return;
	}
	
	check(World);
	check(World == GetWorld());

	if (World->GetGameState() != nullptr)
	{
		LoadPrimaryGameExperience(World->GetGameState());
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &ThisClass::LoadPrimaryGameExperience);
	}
}

void UGameplayExperienceSubsystem::LoadPrimaryGameExperience(AGameStateBase* GameState)
{
	check(GameState);
	check(GameState->GetWorld() == GetWorld());

	GetWorld()->GameStateSetEvent.RemoveAll(this);
	
	const FPrimaryAssetId ExperienceId = LocateGameplayExperience(GetWorld());
	LoadGameExperience(ExperienceId);
}

FPrimaryAssetId UGameplayExperienceSubsystem::LocateGameplayExperience(const UWorld* World) const
{
	const AGameModeBase* GameMode = World->GetAuthGameMode();
	check(GameMode);

	FPrimaryAssetId ExperienceID{};
	const FPrimaryAssetType DefaultAssetType = GetDefault<UGameExperienceSettings>()->DefaultExperienceType->GetFName();

	if (UGameplayStatics::HasOption(GameMode->OptionsString, TEXT("Experience")))
	{
		const FString ExperienceStr = UGameplayStatics::ParseOption(GameMode->OptionsString, TEXT("Experience"));
		ExperienceID = FPrimaryAssetId(DefaultAssetType, FName(*ExperienceStr));
		return ExperienceID;
	}

	FString ExperienceStr;
	if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceStr))
	{
		ExperienceID = FPrimaryAssetId::ParseTypeAndName(ExperienceStr);
		if (!ExperienceID.PrimaryAssetType.IsValid())
		{
			ExperienceID = FPrimaryAssetId(DefaultAssetType, FName(*ExperienceStr));
		}
		return ExperienceID;
	}

	// try core classes for being game experience provider, from most replaceable to least replaceable
	// - World Settings
	// - Game Mode
	// - Game State
	// - Game Instance

	if (IGameplayExperienceProvider::GetGameExperience(World->GetWorldSettings(), ExperienceID))
	{
		return ExperienceID;
	}
	
	if (IGameplayExperienceProvider::GetGameExperience(GameMode, ExperienceID))
	{
		return ExperienceID;
	}

	if (IGameplayExperienceProvider::GetGameExperience(World->GetGameState(), ExperienceID))
	{
		return ExperienceID;
	}
	
	if (IGameplayExperienceProvider::GetGameExperience(World->GetGameInstance(), ExperienceID))
	{
		return ExperienceID;
	}

	const UGameExperienceSettings* ExperienceSettings = GetDefault<UGameExperienceSettings>();
	if (ExperienceSettings->DefaultExperience.IsValid())
	{
		ExperienceID = ExperienceSettings->DefaultExperience;
		return ExperienceID;
	}
	if (ExperienceSettings->DefaultExperienceType)
	{
		ExperienceID = ExperienceSettings->DefaultExperienceType->GetDefaultObject()->GetPrimaryAssetId();
		return ExperienceID;
	}

	UE_LOG(LogGameExperience, Error, TEXT("Failed to locate game experience."));
	return ExperienceID;
}
