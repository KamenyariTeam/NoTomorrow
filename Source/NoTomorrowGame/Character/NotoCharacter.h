// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "NotoCharacter.generated.h"

class UInputComponent;
class UGameplayCameraComponent;

/** Player avatar used by No Tomorrow. */
UCLASS(Config = Game, BlueprintType)
class NOTOMORROWGAME_API ANotoCharacter : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANotoCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void UnPossessed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeAbilitySystem();
	void UninitializeAbilitySystem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;
};
