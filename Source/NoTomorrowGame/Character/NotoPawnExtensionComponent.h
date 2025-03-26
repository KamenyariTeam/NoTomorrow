// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "NotoPawnExtensionComponent.generated.h"

class UGameFrameworkComponentManager;
class UNotoPawnData;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;

/**
 * UNotoPawnExtensionComponent
 *
 * Component added to Pawn actors to coordinate initialization.
 * GAS-related functionality has been removed.
 */
UCLASS()
class NOTOMORROWGAME_API UNotoPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UNotoPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	/** The name for this feature. */
	static const FName NAME_ActorFeatureName;

	// IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	// End IGameFrameworkInitStateInterface interface

	/** Returns the Pawn Extension Component for the specified actor, if one exists. */
	UFUNCTION(BlueprintPure, Category = "Noto|Pawn")
	static UNotoPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UNotoPawnExtensionComponent>() : nullptr;
	}

	/** Gets the pawn data for this pawn. */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** Sets the pawn data for this pawn. */
	void SetPawnData(const UNotoPawnData* InPawnData);

	/** Called when the pawn's controller changes. */
	void HandleControllerChanged();

	/** Called when the pawn's player state is replicated. */
	void HandlePlayerStateReplicated();

	/** Called when the input component is set up. */
	void SetupPlayerInputComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called when PawnData is replicated. */
	UFUNCTION()
	void OnRep_PawnData();

	// Delegate fired when pawn data is initialized.
	FSimpleMulticastDelegate OnPawnDataInitialized;

protected:
	/** Pawn data used to define this pawn. */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Noto|Pawn")
	TObjectPtr<const UNotoPawnData> PawnData;

	// (Note: Removed AbilitySystemComponent pointer and related methods.)
};
