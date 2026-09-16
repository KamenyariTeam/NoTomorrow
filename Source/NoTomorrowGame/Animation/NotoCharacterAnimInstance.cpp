// © 2025 Kamenyari. All rights reserved.

#include "Animation/NotoCharacterAnimInstance.h"

#include "Animation/NotoHeldItemAnimationProfile.h"
#include "Character/NotoCharacter.h"
#include "Character/NotoEquippedItemActor.h"
#include "Character/NotoEquippedItemComponent.h"
#include "Character/NotoPlayerPawnComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoCharacterAnimInstance)

namespace
{
	bool HasVariantChanged(const FNotoHeldItemAnimSnapshot& Previous, const FNotoHeldItemAnimSnapshot& Current)
	{
		return Previous.bHasHeldItemAnimation != Current.bHasHeldItemAnimation
			|| Previous.PoseMode != Current.PoseMode
			|| Previous.bCrouched != Current.bCrouched
			|| Previous.StanceSequence != Current.StanceSequence
			|| Previous.AimOffset != Current.AimOffset;
	}
}

void UNotoCharacterAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	FNotoHeldItemAnimSnapshot ResolvedSnapshot;
	ResolveHeldItemAnimSnapshot(ResolvedSnapshot);
	if (HasVariantChanged(HeldItemAnimSnapshot, ResolvedSnapshot))
	{
		PreviousHeldItemAnimSnapshot = HeldItemAnimSnapshot;
		ResolvedSnapshot.VariantRevision = HeldItemAnimSnapshot.VariantRevision + 1;
		HeldItemAnimSnapshot = ResolvedSnapshot;
		HeldItemAnimTransitionAlpha = 0.0f;
	}
	else
	{
		ResolvedSnapshot.VariantRevision = HeldItemAnimSnapshot.VariantRevision;
		HeldItemAnimSnapshot = ResolvedSnapshot;
		const float BlendTime = FMath::Max(KINDA_SMALL_NUMBER, HeldItemAnimSnapshot.BlendTime);
		HeldItemAnimTransitionAlpha = FMath::Min(1.0f, HeldItemAnimTransitionAlpha + DeltaSeconds / BlendTime);
	}
}

void UNotoCharacterAnimInstance::ResolveHeldItemAnimSnapshot(FNotoHeldItemAnimSnapshot& OutSnapshot) const
{
	OutSnapshot = {};

	const ANotoCharacter* Character = Cast<ANotoCharacter>(TryGetPawnOwner());
	const UNotoEquippedItemComponent* EquippedItemComponent = Character ? Character->GetEquippedItemComponent() : nullptr;
	const UNotoHeldItemAnimationProfile* Profile = EquippedItemComponent ? EquippedItemComponent->GetDisplayedHeldItemAnimationProfile() : nullptr;
	if (!Character || !Profile)
	{
		return;
	}

	const UNotoPlayerPawnComponent* PlayerPawnComponent = Character->FindComponentByClass<UNotoPlayerPawnComponent>();
	const ENotoHeldItemPoseMode PoseMode = PlayerPawnComponent ? PlayerPawnComponent->GetHeldItemPoseMode() : ENotoHeldItemPoseMode::HipFire;
	const FNotoHeldItemPoseVariant* Variant = Profile->FindPoseVariant(PoseMode, Character->bIsCrouched);
	if (!Variant)
	{
		return;
	}

	OutSnapshot.bHasHeldItemAnimation = true;
	OutSnapshot.PoseMode = PoseMode;
	OutSnapshot.bCrouched = Character->bIsCrouched;
	OutSnapshot.StanceSequence = Variant->StanceSequence;
	OutSnapshot.bHasAimOffset = IsValid(Variant->AimOffset);
	OutSnapshot.AimOffset = Variant->AimOffset;
	OutSnapshot.BlendTime = Variant->BlendTime;
	OutSnapshot.AimOffsetAlpha = Variant->AimOffsetAlpha;
	OutSnapshot.SupportHandIKAlpha = Profile->GetSupportHandIKAlpha();
	OutSnapshot.DisableLeftHandIKAlpha = FMath::Clamp(GetCurveValue(TEXT("DisableLHandIK")), 0.0f, 1.0f);

	USkeletalMeshComponent* CharacterMesh = GetSkelMeshComponent();
	ANotoEquippedItemActor* EquippedActor = EquippedItemComponent->GetDisplayedActor();
	USkeletalMeshComponent* ItemMesh = EquippedActor ? EquippedActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	const FName SupportSocketName = Profile->GetSupportHandSocketName();
	if (!CharacterMesh || !ItemMesh || SupportSocketName.IsNone() || !ItemMesh->DoesSocketExist(SupportSocketName)
		|| CharacterMesh->GetBoneIndex(TEXT("weapon_r")) == INDEX_NONE)
	{
		return;
	}

	const FTransform WeaponBoneWorld = CharacterMesh->GetSocketTransform(TEXT("weapon_r"), RTS_World);
	const FTransform SupportSocketWorld = ItemMesh->GetSocketTransform(SupportSocketName, RTS_World);
	OutSnapshot.bHasSupportHandTarget = true;
	OutSnapshot.SupportHandTargetWeaponSpace = SupportSocketWorld.GetRelativeTransform(WeaponBoneWorld);
	OutSnapshot.SupportHandTargetComponentSpace = SupportSocketWorld.GetRelativeTransform(CharacterMesh->GetComponentTransform());
}
