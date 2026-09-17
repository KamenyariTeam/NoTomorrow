// © 2025 Kamenyari. All rights reserved.

#include "Ragdoll/NotoAnimNotify_TriggerRagdoll.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Ragdoll/NotoRagdollComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoAnimNotify_TriggerRagdoll)

void UNotoAnimNotify_TriggerRagdoll::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}

	if (bStopActiveMontages)
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(MontageBlendOutTime);
		}
	}

	if (UNotoRagdollComponent* RagdollComponent = MeshComp->GetOwner()->FindComponentByClass<UNotoRagdollComponent>())
	{
		RagdollComponent->StartRagdoll(FVector::ZeroVector, InjuryState);
	}
}
