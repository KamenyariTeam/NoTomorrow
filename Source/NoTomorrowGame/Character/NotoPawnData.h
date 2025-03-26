// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "NotoPawnData.generated.h"

class APawn;
class UNotoInputConfig;
class UNotoCameraMode;

/**
 * UNotoPawnData
 *
 * Non-mutable data asset that defines a Pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Noto Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class NOTOMORROWGAME_API UNotoPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UNotoPawnData(const FObjectInitializer& ObjectInitializer);

	// Class to instantiate for this pawn.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UNotoInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Camera")
	TSubclassOf<UNotoCameraMode> DefaultCameraMode;
};
