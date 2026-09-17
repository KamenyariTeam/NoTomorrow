// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Traversal/NotoTraversalTypes.h"
#include "NotoTraversalComponent.generated.h"

class ACharacter;
class UAnimMontage;
class UAnimInstance;
class UCharacterMovementComponent;
class UMotionWarpingComponent;

UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoTraversalComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Noto|Traversal")
	bool TryTraversalAction(const FNotoTraversalCheckInputs& Inputs, FNotoTraversalCheckResult& OutResult);

	UFUNCTION(BlueprintPure, Category = "Noto|Traversal")
	FNotoTraversalMontageChooserInput BuildMontageChooserInput(const FNotoTraversalCheckResult& Result, const FPoseHistoryReference& PoseHistory) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Traversal")
	FNotoTraversalCheckResult ApplyMontageChooserOutput(FNotoTraversalCheckResult Result, const FNotoTraversalMontageChooserOutput& Selection) const;

	UFUNCTION(BlueprintCallable, Category = "Noto|Traversal")
	bool StartTraversal(const FNotoTraversalCheckResult& Result);

	UFUNCTION(BlueprintCallable, Category = "Noto|Traversal")
	void StopTraversal(float BlendOutTime = 0.2f);

	UFUNCTION(BlueprintPure, Category = "Noto|Traversal")
	bool IsTraversalActive() const { return bTraversalActive; }

	UFUNCTION(BlueprintPure, Category = "Noto|Traversal")
	FNotoTraversalCheckResult GetTraversalResult() const { return TraversalResult; }

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool QueryTraversalSurface(const FNotoTraversalCheckInputs& Inputs, FNotoTraversalCheckResult& OutResult) const;
	void SetWarpTargets(const FNotoTraversalCheckResult& Result) const;
	void SetTraversalActionActive(bool bActive);
	void FinishTraversal();
	void RestoreReplicationBehavior();
	void HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	ACharacter* GetOwnerCharacter() const;
	UMotionWarpingComponent* GetMotionWarpingComponent() const;
	UCharacterMovementComponent* GetCharacterMovement() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Traversal", Meta = (AllowPrivateAccess = "true"))
	FNotoTraversalCheckResult TraversalResult;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Traversal", Meta = (AllowPrivateAccess = "true"))
	bool bTraversalActive = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveMontage = nullptr;

	FTimerHandle ReplicationRestoreTimer;
};
