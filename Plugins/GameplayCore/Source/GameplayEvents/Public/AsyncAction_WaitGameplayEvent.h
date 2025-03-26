#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameplayEventHandle.h"
#include "GameplayTagContainer.h"

#include "AsyncAction_WaitGameplayEvent.generated.h"

class UWorld;
class UScriptStruct;
class UAsyncAction_WaitGameplayEvent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameplayEventDelegate, UAsyncAction_WaitGameplayEvent*, ProxyObject, FGameplayTag, ActualChannel);

UCLASS(BlueprintType, meta = (HasDedicatedAsyncNode))
class GAMEPLAYEVENTS_API UAsyncAction_WaitGameplayEvent: public UCancellableAsyncAction
{
	GENERATED_BODY()
public:
	
	/**
	 * Asynchronously waits for gameplay event defined by event type to be broadcast on a specified channel
	 * @param WorldContextObject world context
	 * @param Channel channel to listen
	 * @param EventType type of event
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Events", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_WaitGameplayEvent* WaitForGameplayEvent(const UObject* WorldContextObject, FGameplayTag Channel, const UScriptStruct* EventType);

	//~Begin CancellableAsyncAction interface
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	//~End CancellableAsyncAction interface
	
	UPROPERTY(BlueprintAssignable)
	FGameplayEventDelegate Received;

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Events", meta = (CustomStructureParam = "OutEventData"))
	bool GetEventData(UPARAM(ref) int32& OutEventData);

	DECLARE_FUNCTION(execGetEventData);
	
private:

	void HandleEventReceived(FGameplayTag ActualChannel, const void* Event);

	const void* ReceivedEventData = nullptr;

	TWeakObjectPtr<const UWorld> World;
	FGameplayTag Channel;
	TWeakObjectPtr<const UScriptStruct> EventType;

	FGameplayEventHandle EventHandle;
};
