// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/NotoHeldItemAnimationTypes.h"
#include "NotoCharacterAnimInstance.generated.h"

/** Game-thread bridge from equipped-item presentation to a value-only AnimGraph snapshot. */
UCLASS(Blueprintable, Transient)
class NOTOMORROWGAME_API UNotoCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item", Meta = (BlueprintThreadSafe))
	const FNotoHeldItemAnimSnapshot& GetHeldItemAnimSnapshot() const { return HeldItemAnimSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item", Meta = (BlueprintThreadSafe))
	const FNotoHeldItemAnimSnapshot& GetPreviousHeldItemAnimSnapshot() const { return PreviousHeldItemAnimSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item", Meta = (BlueprintThreadSafe))
	float GetHeldItemAnimTransitionAlpha() const { return HeldItemAnimTransitionAlpha; }

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	void ResolveHeldItemAnimSnapshot(FNotoHeldItemAnimSnapshot& OutSnapshot) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	FNotoHeldItemAnimSnapshot HeldItemAnimSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	FNotoHeldItemAnimSnapshot PreviousHeldItemAnimSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	float HeldItemAnimTransitionAlpha = 1.0f;
};
