// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "NotoCharacter.generated.h"

class UInputComponent;
class UGameplayCameraComponent;
class UNotoFirearmComponent;
class UNotoEquippedItemComponent;
class UNotoHealthComponent;
class UNotoInteractionComponent;
class UNotoPlayerPawnComponent;
class UNotoRagdollComponent;
class UNotoTraversalComponent;
class UMotionWarpingComponent;

/** Player avatar used by No Tomorrow. */
UCLASS(Config = Game, BlueprintType)
class NOTOMORROWGAME_API ANotoCharacter : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANotoCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ACharacter interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void UnPossessed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of ACharacter interface

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Noto|Health")
	UNotoHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	UNotoFirearmComponent* GetFirearmComponent() const { return FirearmComponent; }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	UNotoEquippedItemComponent* GetEquippedItemComponent() const { return EquippedItemComponent; }

	UFUNCTION(BlueprintPure, Category = "Noto|Traversal")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	UFUNCTION(BlueprintPure, Category = "Noto|Traversal")
	UNotoTraversalComponent* GetTraversalComponent() const { return TraversalComponent; }

	UFUNCTION(BlueprintPure, Category = "Noto|Ragdoll")
	UNotoRagdollComponent* GetRagdollComponent() const { return RagdollComponent; }

	UFUNCTION(BlueprintPure, Category = "Noto|Character")
	UNotoPlayerPawnComponent* GetPlayerPawnComponent() const { return PlayerPawnComponent; }

protected:
	void InitializeAbilitySystem();
	void UninitializeAbilitySystem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Traversal", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Traversal", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoTraversalComponent> TraversalComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Ragdoll", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoRagdollComponent> RagdollComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Interaction", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Health", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Weapon", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoFirearmComponent> FirearmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Weapon", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoEquippedItemComponent> EquippedItemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoPlayerPawnComponent> PlayerPawnComponent;
};
