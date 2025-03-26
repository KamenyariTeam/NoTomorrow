// © 2025 Kamenyari. All rights reserved.

#include "NotoGameEngine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoGameEngine)

class IEngineLoop;

UNotoGameEngine::UNotoGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNotoGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}
