// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameplayEffect.h"
#include "NotoDamageEffect.generated.h"

/** Instant set-by-caller damage routed through UNotoHealthSet::IncomingDamage. */
UCLASS()
class NOTOMORROWGAME_API UNotoDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UNotoDamageEffect();
};
