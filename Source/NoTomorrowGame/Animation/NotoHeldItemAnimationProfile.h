// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Animation/NotoHeldItemAnimationTypes.h"
#include "Engine/DataAsset.h"
#include "NotoHeldItemAnimationProfile.generated.h"

/** Immutable animation-presentation configuration for one held item. */
UCLASS(BlueprintType)
class NOTOMORROWGAME_API UNotoHeldItemAnimationProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	const FNotoHeldItemPoseVariant* FindPoseVariant(ENotoHeldItemPoseMode PoseMode, bool bCrouched) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item")
	FName GetSupportHandSocketName() const { return SupportHandSocketName; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item")
	float GetSupportHandIKAlpha() const { return SupportHandIKAlpha; }

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Held Item", Meta = (AllowPrivateAccess = "true", TitleProperty = "PoseMode"))
	TArray<FNotoHeldItemPoseVariant> PoseVariants;

	/** A socket on the equipped presentation mesh, normally Grip_L. It is intentionally per mesh/profile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Held Item|IK", Meta = (AllowPrivateAccess = "true"))
	FName SupportHandSocketName = TEXT("Grip_L");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Held Item|IK", Meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float SupportHandIKAlpha = 1.0f;
};
