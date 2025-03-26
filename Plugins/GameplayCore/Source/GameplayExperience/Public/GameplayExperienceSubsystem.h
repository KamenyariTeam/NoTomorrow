#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "GameplayExperienceSubsystem.generated.h"

class UGameplayExperience;

UCLASS(Config = Game, DefaultConfig)
class GAMEPLAYEXPERIENCE_API UGameExperienceSettings: public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UGameExperienceSettings();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, meta = (AllowedTypes = "GameplayExperience"))
	FPrimaryAssetId DefaultExperience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config)
	TSubclassOf<UGameplayExperience> DefaultExperienceType;
};

UCLASS()
class GAMEPLAYEXPERIENCE_API UGameplayExperienceSubsystem: public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	static UGameplayExperienceSubsystem* Get(const UObject* WorldContextObject);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void LoadGameExperience(FPrimaryAssetId ExperienceId);
	
protected:

	virtual void LoadPrimaryGameExperience(AGameStateBase* GameState);
	void HandleInitGame(const FActorsInitializedParams& Params);

	FPrimaryAssetId LocateGameplayExperience(const UWorld* World) const;
};
