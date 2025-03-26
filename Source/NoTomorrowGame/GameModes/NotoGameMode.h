// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "NotoGameMode.generated.h"

class UGameplayExperience;
class UNotoPawnData;
/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNotoGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * Primary game mode for No Tomorrow.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	ANotoGameMode(const FObjectInitializer& Initializer);

	UFUNCTION(BlueprintCallable, Category = "Noto|Pawn")
	const UNotoPawnData* GetPawnDataForController(const AController* InController) const;
	
	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void InitGameState() override;
	//~End of AGameModeBase interface
	
	// Delegate called on player initialization, described above 
	FOnNotoGameModePlayerInitialized OnGameModePlayerInitialized;
protected:
	void OnExperienceLoaded();
	bool IsExperienceLoaded() const;
};
