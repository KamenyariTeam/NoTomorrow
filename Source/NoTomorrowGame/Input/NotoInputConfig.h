// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NotoInputConfig.generated.h"

class UInputAction;

/**
 * FNotoInputAction
 *
 * Struct used to map an input action to a gameplay tag.
 */
USTRUCT(BlueprintType)
struct FNotoInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * UNotoInputConfig
 *
 * Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const)
class NOTOMORROWGAME_API UNotoInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UNotoInputConfig(const FObjectInitializer& ObjectInitializer);

	/**
	 * Finds an input action assigned to a gameplay tag.
	 * @param InputTag - The tag to search for.
	 * @param bLogNotFound - Whether to log an error if not found.
	 * @return The matching UInputAction if found, otherwise nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	// List of input actions used by the owner. These input actions are mapped to gameplay tags.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FNotoInputAction> InputActions;
};
