// © 2025 Kamenyari. All rights reserved.

#include "NotoInputConfig.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogNotoInput, Log, All);

UNotoInputConfig::UNotoInputConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UInputAction* UNotoInputConfig::FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FNotoInputAction& Action : InputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogNotoInput, Error, TEXT("Can't find InputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
