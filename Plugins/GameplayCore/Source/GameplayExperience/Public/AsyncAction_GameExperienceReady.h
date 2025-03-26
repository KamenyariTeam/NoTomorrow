#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_GameExperienceReady.generated.h"

class UGameExperienceManagerComponent;
class UGameplayExperience;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameExperienceReadyDelegate);

/**
 * Asynchronously waits for game experience to be loaded and ready. Calls OnReady immediately if experience has already been loaded
 */
UCLASS()
class GAMEPLAYEXPERIENCE_API UAsyncAction_GameExperienceReady: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	
	/**
	 * Asynchronously waits for game experience to be loaded and ready. Calls OnReady immediately if experience has already been loaded
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DefaultToSelf = "WorldContextObject"))
	static UAsyncAction_GameExperienceReady* WaitForGameExperienceReady(UObject* WorldContextObject);
	
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FGameExperienceReadyDelegate OnReady;

private:

	void HandleGameStateSet(AGameStateBase* GameState);
	void HandleGameExperienceReady();
	
	TWeakObjectPtr<UWorld> World;
};
