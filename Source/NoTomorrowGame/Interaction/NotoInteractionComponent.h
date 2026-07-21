// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "NotoInteractionComponent.generated.h"

class AActor;
class AController;
class APawn;
class UPrimitiveComponent;
class USphereComponent;

/** Selects nearby interactables nearest the player's gameplay cursor. */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoInteractionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Noto|Interaction")
	void TryInteract();

	UFUNCTION(BlueprintPure, Category = "Noto|Interaction")
	AActor* GetSelectedInteractable() const;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	void RefreshInteractionEnabled();
	void RefreshSelection();
	void SetSelectedInteractable(AActor* NewSelectedInteractable);

	UPROPERTY(VisibleAnywhere, Category = "Noto|Interaction")
	TObjectPtr<USphereComponent> InteractionRange;

	UPROPERTY(EditDefaultsOnly, Category = "Noto|Interaction", Meta = (ClampMin = "0.0", Units = "cm"))
	float InteractionRadius = 150.0f;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SelectedInteractable;

	TSet<TWeakObjectPtr<AActor>> NearbyInteractables;
};
