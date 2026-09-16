// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NotoHeldItemAnimationTypes.generated.h"

class UAnimSequenceBase;
class UBlendSpace;

/** High-level local presentation mode for a held item. Gameplay tags remain the action and item identity contract. */
UENUM(BlueprintType)
enum class ENotoHeldItemPoseMode : uint8
{
	HipFire,
	ADS,
	Sprint,
	Carry
};

/** One authored continuous-pose variant selected by the active held-item profile. */
USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoHeldItemPoseVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	ENotoHeldItemPoseMode PoseMode = ENotoHeldItemPoseMode::HipFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	bool bCrouched = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	TObjectPtr<UAnimSequenceBase> StanceSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item")
	TObjectPtr<UBlendSpace> AimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", Meta = (ClampMin = "0.0", Units = "s"))
	float BlendTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", Meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimOffsetAlpha = 1.0f;
};

/** Value-only presentation data resolved on the game thread and consumed by the animation graph. */
USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoHeldItemAnimSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	bool bHasHeldItemAnimation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	ENotoHeldItemPoseMode PoseMode = ENotoHeldItemPoseMode::HipFire;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	bool bCrouched = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	TObjectPtr<UAnimSequenceBase> StanceSequence;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	bool bHasAimOffset = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	TObjectPtr<UBlendSpace> AimOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	float BlendTime = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	float AimOffsetAlpha = 1.0f;

	/** Changes only when the selected continuous-pose variant changes, not when the support socket moves. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item")
	int32 VariantRevision = 0;

	/** Transform of the presentation mesh's authored support socket, expressed in Manny's weapon_r bone space. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item|IK")
	bool bHasSupportHandTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item|IK")
	FTransform SupportHandTargetWeaponSpace = FTransform::Identity;

	/** Same support socket transform in the character mesh's component space, used for final hand orientation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item|IK")
	FTransform SupportHandTargetComponentSpace = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item|IK")
	float SupportHandIKAlpha = 1.0f;

	/** Authored character-animation curve value. One disables support-hand IK during actions such as reload. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held Item|IK")
	float DisableLeftHandIKAlpha = 0.0f;
};
