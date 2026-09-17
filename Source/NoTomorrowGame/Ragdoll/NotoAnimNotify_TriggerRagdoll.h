// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "Ragdoll/NotoRagdollTypes.h"
#include "NotoAnimNotify_TriggerRagdoll.generated.h"

UCLASS(DisplayName = "Trigger Noto Ragdoll")
class NOTOMORROWGAME_API UNotoAnimNotify_TriggerRagdoll : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bStopActiveMontages = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", Meta = (ClampMin = "0.0"))
	float MontageBlendOutTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	ENotoRagdollInjuryState InjuryState = ENotoRagdollInjuryState::None;
};
