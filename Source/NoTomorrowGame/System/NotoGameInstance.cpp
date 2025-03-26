// © 2025 Kamenyari. All rights reserved.


#include "NotoGameInstance.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Development/NotoGameplayTags.h"


void UNotoGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(NotoGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(NotoGameplayTags::InitState_DataAvailable, false, NotoGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(NotoGameplayTags::InitState_DataInitialized, false, NotoGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(NotoGameplayTags::InitState_GameplayReady, false, NotoGameplayTags::InitState_DataInitialized);
	}
}
