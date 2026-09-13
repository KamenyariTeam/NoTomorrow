// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NotoEquippedItemActor.generated.h"

class UAnimMontage;
class UNotoItemDefinition;
class USceneComponent;

/** Presentation actor attached while an inventory item is active. Concrete Blueprint classes own their components. */
UCLASS(Blueprintable)
class NOTOMORROWGAME_API ANotoEquippedItemActor : public AActor
{
	GENERATED_BODY()

public:
	ANotoEquippedItemActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeFromItem(UNotoItemDefinition* InDefinition, FGuid InItemInstanceId);
	bool PlayItemAction(FGameplayTag ActionTag, UAnimMontage* ItemMontage);

	UFUNCTION(BlueprintPure, Category = "Noto|Equipment")
	UNotoItemDefinition* GetItemDefinition() const { return ItemDefinition; }

	UFUNCTION(BlueprintPure, Category = "Noto|Equipment")
	FGuid GetItemInstanceId() const { return ItemInstanceId; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Noto|Equipment", Meta = (DisplayName = "On Item Action"))
	void ReceiveItemAction(FGameplayTag ActionTag);

private:
	UPROPERTY(VisibleAnywhere, Category = "Noto|Equipment")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Transient)
	TObjectPtr<UNotoItemDefinition> ItemDefinition;

	FGuid ItemInstanceId;
};
