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
 * Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class NOTOMORROWGAME_API UNotoInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UNotoInputComponent(const FObjectInitializer& ObjectInitializer);

	/** Optionally add custom logic to add input mappings from the config. */
	void AddInputMappings(const UNotoInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	/** Optionally remove input mappings that were added. */
	void RemoveInputMappings(const UNotoInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	/**
	 * Template helper to bind a native action by gameplay tag.
	 * Looks up the input action in the config and binds it if found.
	 */
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UNotoInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound = true)
	{
		check(InputConfig);
		if (const UInputAction* IA = InputConfig->FindInputActionForTag(InputTag, bLogIfNotFound))
		{
			BindAction(IA, TriggerEvent, Object, Func);
		}
	}

	/**
	 * Template helper to bind actions for ability-like inputs.
	 * Iterates over all input actions in the config and binds both Triggered and Completed events.
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UNotoInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
	{
		check(InputConfig);
		for (const FNotoInputAction& Action : InputConfig->InputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				if (PressedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
				}
				if (ReleasedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
				}
			}
		}
	}

	/** Removes bindings for the given bind handles. */
	void RemoveBinds(TArray<uint32>& BindHandles);
};
