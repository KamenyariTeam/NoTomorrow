// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Interaction/NotoInteractableActor.h"
#include "NotoTestPickup.generated.h"

/** Temporary interactable that validates selection and interaction before inventory exists. */
UCLASS(Blueprintable)
class NOTOMORROWGAME_API ANotoTestPickup : public ANotoInteractableActor
{
	GENERATED_BODY()

public:
	ANotoTestPickup();

	virtual void Interact_Implementation(APawn* Interactor, const FNotoInteractionRequest& Request) override;
};
