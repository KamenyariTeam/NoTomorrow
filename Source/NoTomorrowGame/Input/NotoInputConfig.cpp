// © 2025 Kamenyari. All rights reserved.

#include "NotoInputConfig.h"

#include "Development/NotoLogChannels.h"
#include "Logging/LogMacros.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogNotoInput, Log, All);

const UInputAction* UNotoInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FNotoTaggedInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogNoto, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UNotoInputConfig::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	TSet<FGameplayTag> SeenTags;

	for (int32 Index = 0; Index < NativeInputActions.Num(); ++Index)
	{
		const FNotoTaggedInputAction& Entry = NativeInputActions[Index];
		if (!Entry.InputAction)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(NSLOCTEXT("NotoInputConfig", "NullInputAction", "NativeInputActions[{0}] has no InputAction."), Index));
		}

		if (!Entry.InputTag.IsValid())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(NSLOCTEXT("NotoInputConfig", "InvalidInputTag", "NativeInputActions[{0}] has an invalid InputTag."), Index));
			continue;
		}

		if (SeenTags.Contains(Entry.InputTag))
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(NSLOCTEXT("NotoInputConfig", "DuplicateInputTag", "NativeInputActions[{0}] duplicates InputTag '{1}'."), Index, FText::FromName(Entry.InputTag.GetTagName())));
			continue;
		}

		SeenTags.Add(Entry.InputTag);
	}

	return Result;
}
#endif
