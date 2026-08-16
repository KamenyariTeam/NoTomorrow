// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "NotoInteractable.generated.h"

class APawn;

/** Per-input request data passed to an interactable. Unused fields are ignored by interactables that do not need them. */
USTRUCT(BlueprintType)
struct FNotoInteractionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Noto|Interaction")
	bool bUseAlternateInteraction = false;
};

/** Contract used by the player interaction selector. */
UINTERFACE(BlueprintType)
class UNotoInteractable : public UInterface
{
	GENERATED_BODY()
};

class NOTOMORROWGAME_API INotoInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Noto|Interaction")
	bool CanInteract(APawn* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Noto|Interaction")
	void Interact(APawn* Interactor, const FNotoInteractionRequest& Request);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Noto|Interaction")
	void SetInteractionHighlighted(bool bHighlighted);
};
