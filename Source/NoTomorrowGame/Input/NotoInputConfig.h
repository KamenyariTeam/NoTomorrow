// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
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
 *	Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const)
class UNotoInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UNotoInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Noto|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	// List of input actions used by the owner. These input actions are mapped to a gameplay tag and must be manually bound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FNotoInputAction> NativeInputActions;
};