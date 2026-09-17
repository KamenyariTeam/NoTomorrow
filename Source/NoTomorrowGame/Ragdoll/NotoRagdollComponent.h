// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Combat/NotoHealthSet.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Ragdoll/NotoRagdollTypes.h"
#include "NotoRagdollComponent.generated.h"

class ACharacter;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UNotoHealthComponent;
class USkeletalMeshComponent;

UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoRagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoRagdollComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Noto|Ragdoll")
	bool StartRagdoll(FVector Impulse = FVector::ZeroVector, ENotoRagdollInjuryState InjuryState = ENotoRagdollInjuryState::None);

	UFUNCTION(BlueprintCallable, Category = "Noto|Ragdoll")
	void StopRagdoll();

	UFUNCTION(BlueprintCallable, Category = "Noto|Ragdoll")
	bool ExitRagdollForRecovery();

	UFUNCTION(BlueprintPure, Category = "Noto|Ragdoll")
	bool IsRagdollActive() const { return RagdollSnapshot.bIsRagdoll; }

	UFUNCTION(BlueprintPure, Category = "Noto|Ragdoll")
	const FNotoRagdollSnapshot& GetRagdollSnapshot() const { return RagdollSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Ragdoll")
	bool CanRecover() const { return RagdollSnapshot.bCanRecover; }

	UFUNCTION(BlueprintCallable, Category = "Noto|Ragdoll")
	void SetRagdollInjuryState(ENotoRagdollInjuryState InjuryState) { RagdollSnapshot.InjuryState = InjuryState; }

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleDeathStarted(const FNotoDamageEvent& DamageEvent);

	ACharacter* GetOwnerCharacter() const;
	USkeletalMeshComponent* GetMesh() const;
	UCapsuleComponent* GetCapsule() const;
	UCharacterMovementComponent* GetCharacterMovement() const;
	void UpdateSnapshot(float DeltaTime);
	void RestoreCharacterPhysics();
	void SetRagdollGameplayState(bool bActive);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll", Meta = (AllowPrivateAccess = "true"))
	bool bRagdollOnDeath = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll", Meta = (AllowPrivateAccess = "true"))
	FName RagdollRootBone = TEXT("pelvis");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll", Meta = (AllowPrivateAccess = "true"))
	FName RagdollCollisionProfile = TEXT("Ragdoll");

	/** The ragdoll-root local axis that points out of the character's chest. Tune per skeleton if necessary. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EAxis::Type> RagdollFacingAxis = EAxis::X;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EAxis::Type> RagdollRightAxis = EAxis::Y;

	/** The ragdoll-root local axis running from the pelvis toward the character's forward recovery direction. Tune per skeleton. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EAxis::Type> RagdollRecoveryForwardAxis = EAxis::Z;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true"))
	bool bAlignCharacterToRecoveryFacing = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float FaceUpDotThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinimumRagdollTimeBeforeRecovery = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Ragdoll|Recovery", Meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RecoverySpeedThreshold = 10.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Ragdoll", Meta = (AllowPrivateAccess = "true"))
	FNotoRagdollSnapshot RagdollSnapshot;

	FName PreviousMeshCollisionProfile;
	FName PreviousCapsuleCollisionProfile;
	TEnumAsByte<ECollisionEnabled::Type> PreviousCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_Walking;
	TWeakObjectPtr<UNotoHealthComponent> HealthComponent;
};
