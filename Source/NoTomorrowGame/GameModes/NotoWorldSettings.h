// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameplayExperienceProvider.h"
#include "GameFramework/WorldSettings.h"
#include "NotoWorldSettings.generated.h"

class UNotoGameplayExperience;

/**
 * ANotoWorldSettings
 *
 * The default world settings used to define the gameplay experience for this map.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoWorldSettings : public AWorldSettings, public IGameplayExperienceProvider
{
	GENERATED_BODY()

public:
	ANotoWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	
	//~Begin IGameplayExperienceProvider interface
	virtual FPrimaryAssetId GetGameExperience() const override;
	//~End IGameplayExperienceProvider interface

protected:
	// The experience asset used when starting this map.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowedClass = "NotoGameplayExperience"))
	FPrimaryAssetId GameplayExperience;

public:
#if WITH_EDITORONLY_DATA
	// When true, forces standalone net mode when playing in the editor.
	UPROPERTY(EditDefaultsOnly, Category = "PIE")
	bool bForceStandaloneNetMode = false;
#endif
};
