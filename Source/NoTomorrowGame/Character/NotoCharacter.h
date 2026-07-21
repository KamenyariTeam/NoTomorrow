// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "NotoCharacter.generated.h"

class UInputComponent;
class UGameplayCameraComponent;
class UNotoInteractionComponent;
class UNotoPlayerPawnComponent;

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

protected:

	void InitializeAbilitySystem();
	void UninitializeAbilitySystem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Interaction", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoPlayerPawnComponent> PlayerPawnComponent;
};
