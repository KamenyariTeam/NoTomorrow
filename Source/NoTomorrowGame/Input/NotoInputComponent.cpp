// © 2025 Kamenyari. All rights reserved.

#include "NotoInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "NotoInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoInputComponent)

UNotoInputComponent::UNotoInputComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNotoInputComponent::AddInputMappings(const UNotoInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// Insert custom logic if you need to add mapping contexts from your InputConfig.
}

void UNotoInputComponent::RemoveInputMappings(const UNotoInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// Insert custom logic if you need to remove mapping contexts that were added.
}

void UNotoInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
