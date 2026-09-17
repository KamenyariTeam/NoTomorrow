// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "NotoTraversalTypes.h"
#include "UObject/Interface.h"
#include "NotoTraversalSurface.generated.h"

UINTERFACE(BlueprintType)
class UNotoTraversalSurface : public UInterface
{
	GENERATED_BODY()
};

class NOTOMORROWGAME_API INotoTraversalSurface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Noto|Traversal")
	void GetTraversalLedgeData(const FVector& HitLocation, const FVector& CharacterLocation, FNotoTraversalLedgeData& OutLedgeData) const;
};
