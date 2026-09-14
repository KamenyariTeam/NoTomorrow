// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NotoLocomotionCVarLibrary.generated.h"

USTRUCT(BlueprintType)
struct FNotoLocomotionCVarValues
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float OffsetRootTranslationRadius = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bOffsetRootBoneEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	int32 PoseSearchDatabaseLod = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bDrawShapes = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bDrawStates = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bDrawGraphs = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bUseThreadSafeUpdate = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bUseExperimentalStateMachine = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	int32 LocomotionSetup = 0;
};

UCLASS()
class NOTOMORROWGAME_API UNotoLocomotionCVarLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Read the project locomotion development controls once on the game thread. */
	UFUNCTION(BlueprintPure, Category = "Noto|Locomotion")
	static FNotoLocomotionCVarValues GetLocomotionCVarValues();
};
