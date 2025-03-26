// © 2025 Kamenyari. All rights reserved.

#include "NotoWorldSettings.h"
#include "Engine/AssetManager.h"
#include "EngineUtils.h"
#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#include "Development/NotoLogChannels.h"
#include "GameFramework/PlayerStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoWorldSettings)

ANotoWorldSettings::ANotoWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void ANotoWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			MapCheck.Warning()
				->AddToken(FUObjectToken::Create(PlayerStart))
				->AddToken(FTextToken::Create(FText::FromString("is a basic APlayerStart; consider using a custom player start if required.")));
		}
	}
}

FPrimaryAssetId ANotoWorldSettings::GetGameExperience() const
{
	return GameplayExperience;
}
#endif
