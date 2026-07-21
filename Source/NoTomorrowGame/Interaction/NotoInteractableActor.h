// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "NotoInteractable.h"
#include "NotoInteractableActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/** Reusable base for world objects selectable by UNotoInteractionComponent. */
UCLASS(Abstract, Blueprintable)
class NOTOMORROWGAME_API ANotoInteractableActor : public AActor, public INotoInteractable
{
	GENERATED_BODY()

public:
	ANotoInteractableActor();

	virtual bool CanInteract_Implementation(APawn* Interactor) const override;
	virtual void SetInteractionHighlighted_Implementation(bool bHighlighted) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Interaction")
	TObjectPtr<UStaticMeshComponent> Visual;

	/** Query-only volume; resize it in a child Blueprint to match the object. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Interaction")
	TObjectPtr<USphereComponent> InteractionVolume;
};
