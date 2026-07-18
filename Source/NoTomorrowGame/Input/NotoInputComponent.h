// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "EnhancedInputComponent.h"
#include "NotoInputConfig.h"
#include "NotoInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;

/**
 * UNotoInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class UNotoInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UNotoInputComponent(const FObjectInitializer& ObjectInitializer);
	
	template<class UserClass, typename FuncType>
	bool BindNativeAction(const UNotoInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	void RemoveBinds(TArray<uint32>& BindHandles);
};


template<class UserClass, typename FuncType>
bool UNotoInputComponent::BindNativeAction(const UNotoInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	if (InputConfig)
	{
		if (const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
		{
			BindAction(InputAction, TriggerEvent, Object, Func);
			return true;
		}
	}

	return false;
}
