// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/GameEngine.h"

#include "NotoGameEngine.generated.h"

class IEngineLoop;
class UObject;

UCLASS()
class NOTOMORROWGAME_API UNotoGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:

	UNotoGameEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void Init(IEngineLoop* InEngineLoop) override;
};
