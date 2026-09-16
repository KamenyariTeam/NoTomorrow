// © 2025 Kamenyari. All rights reserved.

#include "Animation/NotoHeldItemAnimationProfile.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoHeldItemAnimationProfile)

const FNotoHeldItemPoseVariant* UNotoHeldItemAnimationProfile::FindPoseVariant(const ENotoHeldItemPoseMode PoseMode, const bool bCrouched) const
{
	auto Find = [this](const ENotoHeldItemPoseMode InPoseMode, const bool bInCrouched)
	{
		return PoseVariants.FindByPredicate([InPoseMode, bInCrouched](const FNotoHeldItemPoseVariant& Variant)
		{
			return Variant.PoseMode == InPoseMode && Variant.bCrouched == bInCrouched;
		});
	};

	if (const FNotoHeldItemPoseVariant* ExactVariant = Find(PoseMode, bCrouched))
	{
		return ExactVariant;
	}
	if (const FNotoHeldItemPoseVariant* StandingVariant = Find(PoseMode, false))
	{
		return StandingVariant;
	}
	if (const FNotoHeldItemPoseVariant* HipFireVariant = Find(ENotoHeldItemPoseMode::HipFire, bCrouched))
	{
		return HipFireVariant;
	}
	return Find(ENotoHeldItemPoseMode::HipFire, false);
}

#if WITH_EDITOR
EDataValidationResult UNotoHeldItemAnimationProfile::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	TSet<uint16> AuthoredVariants;

	for (const FNotoHeldItemPoseVariant& Variant : PoseVariants)
	{
		const uint16 VariantKey = static_cast<uint16>(Variant.PoseMode) << 1 | static_cast<uint16>(Variant.bCrouched);
		if (AuthoredVariants.Contains(VariantKey))
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(NSLOCTEXT("NotoHeldItemAnimationProfile", "DuplicateVariant", "Each pose mode/crouch combination may appear only once."));
		}
		AuthoredVariants.Add(VariantKey);
	}

	return Result;
}
#endif
