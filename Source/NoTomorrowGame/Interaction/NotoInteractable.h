// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "UObject/Interface.h"
#include "NotoInteractable.generated.h"

class APawn;

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
	void Interact(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Noto|Interaction")
	void SetInteractionHighlighted(bool bHighlighted);
};
