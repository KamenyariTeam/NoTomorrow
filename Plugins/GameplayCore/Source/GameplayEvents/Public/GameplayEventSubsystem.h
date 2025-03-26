#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayEventHandle.h"

#include "GameplayEventSubsystem.generated.h"

class UGameplayEventSubsystem;

template <typename T>
using TEventCallbackWithTag = TDelegate<void(FGameplayTag, const T&)>;

template <typename T>
using TEventCallback = TDelegate<void(const T&)>;

UENUM(BlueprintType)
enum class ESendEventMode: uint8
{
	Immediate,	// event is sent immediately
	Async,		// event is going to be sent by the end of the frame
};

/**
 * This system allows event senders and receivers to register for events without
 * having to know about each other directly, though they must agree on the format
 * of the message (as a USTRUCT() type).
 *
 *
 * You can get to the message router from the game instance:
 *    UGameInstance::GetSubsystem<UGameplayEventSubsystem>(GameInstance)
 * or directly from anything that has a route to a world:
 *    UGameplayEventSubsystem::Get(WorldContextObject)
 *
 * Note that call order when there are multiple receivers for the same event channel is
 * not guaranteed and can change over time.
 */
UCLASS()
class GAMEPLAYEVENTS_API UGameplayEventSubsystem: public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
	friend class UAsyncAction_WaitGameplayEvent;
	
	using FWrapperCallback = TDelegate<void(FGameplayTag, const void*)>;

	template <typename TEvent>
	struct TChannelEvent
	{
		FGameplayTag OriginalChannel;
		FGameplayTag Channel;
		TEvent Event;
	};
	
	struct FGameplayEventContainer
	{
		using FEventCallback = FWrapperCallback;
		using FChannelEvent = TChannelEvent<const void*>;
		using FCallbackContainer = TMulticastDelegate<void(FGameplayTag, const void*)>;
		
		FDelegateHandle Add(const FGameplayTag& Channel, FEventCallback&& Callback)
		{
			return Channels.FindOrAdd(Channel).Add(Callback);
		}

		void Remove(const FGameplayEventHandle& Handle)
		{
			Channels.FindChecked(Handle.Channel).Remove(Handle.DelegateHandle);
		}

		void RemoveAll(const void* UserObject)
		{
			for (auto& [Channel, Container]: Channels)
			{
				Container.RemoveAll(UserObject);
			}
		}
		
		void Broadcast(FChannelEvent&& Event, ESendEventMode SendMode)
		{
			if (SendMode == ESendEventMode::Async)
			{
				PendingEvents.Add(Forward<FChannelEvent>(Event));
			}
			else if (const FCallbackContainer* Container = Channels.Find(Event.Channel))
			{
				Container->Broadcast(Event.OriginalChannel, Event.Event);
			}
		}

		void BroadcastAsync()
		{
			// make copy of pending events. Any pending events added as a result of BroadcastAsync are handled next time
			TArray<FChannelEvent> PendingEventsCopy{MoveTemp(PendingEvents)};
			
			for (FChannelEvent& Event: PendingEventsCopy)
			{
				if (const FCallbackContainer* Container = Channels.Find(Event.Channel))
				{
					Container->Broadcast(Event.OriginalChannel, Event.Event);
				}
			}
		}

		TMap<FGameplayTag, FCallbackContainer> Channels;
		TArray<FChannelEvent> PendingEvents;
	};
	
	using FGameplayEventContainerPtr = TSharedPtr<FGameplayEventContainer>;
	using FGameplayEventContainerRef = TSharedRef<FGameplayEventContainer>;

public:

	static UGameplayEventSubsystem* Get(const UObject* WorldContextObject);

	//~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End USubsystem interface

	//~Begin TickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(GameplayEventsStats, STATGROUP_Tickables); }
	//~End TickableGameObject interface

	/**
	 * Send event on a specified channel, constructing event data using variadic template
	 * @param Channel event channel to send event to
	 * @param Args arguments to construct the event
	 */
	template <typename TEvent, typename ...TArgs>
	void SendEvent(const FGameplayTag& Channel, TArgs&&... Args)
	{
		TSharedPtr<TEvent> Event = MakeShared<TEvent>(TEvent{Forward<TArgs>(Args)...});
		SendEventInternal(Channel, TBaseStructure<TEvent>::Get(), static_cast<const void*>(Event.Get()), ESendEventMode::Immediate);
	}

	/**
	 * Send event on a specified channel
	 * @param Channel event channel to send event to
	 * @param Event event to send
	 */
	template <typename TEvent>
	void SendEvent(const FGameplayTag& Channel, const TEvent& Event)
	{
		SendEventInternal(Channel, TBaseStructure<TEvent>::Get(), static_cast<const void*>(&Event), ESendEventMode::Immediate);
	}

	/**
	 * Send delayed event on a specified channel, so that subscribers don't receive immediate notification
	 * @note delayed events are processed at the end of the frame
	 * @param Channel event channel to send event to
	 * @param Args arguments to construct event from
	 */
	template <typename TEvent, typename ...TArgs>
	void SendEventAsync(const FGameplayTag& Channel, TArgs&&... Args)
	{
		TSharedPtr<TEvent> Event = MakeShared<TEvent>(TEvent{Forward<TArgs>(Args)...});
		SendEventInternal(Channel, TBaseStructure<TEvent>::Get(), static_cast<const void*>(Event.Get()), ESendEventMode::Async);
	}

	/**
	 * Send delayed event on a specified channel, so that subscribers don't receive immediate notification
	 * @note delayed events are processed at the end of the frame
	 * @param Channel event channel to send event to
	 * @param Event event to send
	 */
	template <typename TEvent>
	void SendEventAsync(const FGameplayTag& Channel, const TEvent& Event)
	{
		SendEventInternal(Channel, TBaseStructure<TEvent>::Get(), static_cast<const void*>(&Event), ESendEventMode::Async);
	}

	/**
	 * Add receiver to a specified channel to receive events of a specified template type
	 * This is a version that receives void(const TEvent&) callback format
	 * @param Channel event channel to listen
	 * @param Callback callback function to call with the event when someone sends it
	 * 
	 * @return gameplay event handle, which can be used to remove this receiver
	 */
	template <typename TEvent>
	FGameplayEventHandle AddReceiver(FGameplayTag Channel, TEventCallback<TEvent>&& Callback)
	{
		const UScriptStruct* EventType = TBaseStructure<TEvent>::Get();
		FGameplayEventHandle EventHandle{this, GenerateNewID(), Channel, EventType};

		FWrapperCallback ThunkCallback = FWrapperCallback::CreateLambda([InnerCallback = Forward<TEventCallback<TEvent>>(Callback)]
			(FGameplayTag Channel, const void* Event)
		{
			if (InnerCallback.IsBound())
			{
				InnerCallback.Execute(*static_cast<const TEvent*>(Event));
			}
		});
		
		EventHandle.DelegateHandle = AddReceiverInternal(Channel, EventType, MoveTemp(ThunkCallback));

		return EventHandle;
	}

	/**
	 * Add receiver to a specified channel to receive events of a specified template type
	 * This is a version that receives void(const TEvent&) callback format
	 * @param Channel event channel to listen
	 * @param Callback callback function to call with the event when someone sends it
	 * 
	 * @return gameplay event handle, which can be used to remove this receiver
	 */
	template <typename TEvent>
	FGameplayEventHandle AddReceiver(FGameplayTag Channel, const TEventCallback<TEvent>& Callback)
	{
		const UScriptStruct* EventType = TBaseStructure<TEvent>::Get();
		FGameplayEventHandle EventHandle{this, GenerateNewID(), Channel, EventType};
		
		FWrapperCallback ThunkCallback = FWrapperCallback::CreateLambda([InnerCallback = Callback]
			(FGameplayTag Channel, const void* Event)
		{
			if (InnerCallback.IsBound())
			{
				InnerCallback.Execute(*static_cast<const TEvent*>(Event));
			}
		});
		
		EventHandle.DelegateHandle = AddReceiverInternal(Channel, EventType, MoveTemp(ThunkCallback));

		return EventHandle;
	}


	/**
	 * Add receiver to a specified channel to receive events of a specified template type
	 * This is a version that receives void(FGameplayTag, const TEvent&) callback format
	 * 
	 * @param Channel event channel to listen
	 * @param Callback callback function to call with the event when someone sends it
	 * 
	 * @return gameplay event handle, which can be used to remove this receiver
	 */
	template <typename TEvent>
	FGameplayEventHandle AddReceiver(FGameplayTag Channel, TEventCallbackWithTag<TEvent>&& Callback)
	{
		const UScriptStruct* EventType = TBaseStructure<TEvent>::Get();
		FGameplayEventHandle EventHandle{this, GenerateNewID(), Channel, EventType};

		FWrapperCallback ThunkCallback = FWrapperCallback::CreateLambda([InnerCallback = Forward<TEventCallbackWithTag<TEvent>>(Callback)]
			(FGameplayTag Channel, const void* Event)
		{
			if (InnerCallback.IsBound())
			{
				InnerCallback.Execute(Channel, *static_cast<const TEvent*>(Event));
			}
		});
		
		EventHandle.DelegateHandle = AddReceiverInternal(Channel, EventType, MoveTemp(ThunkCallback));

		return EventHandle;
	}

	/**
	 * Add receiver to a specified channel to receive events of a specified template type
	 * This is a version that receives void(FGameplayTag, const TEvent&) callback format
	 * 
	 * @param Channel event channel to listen
	 * @param Callback callback function to call with the event when someone sends it
	 * 
	 * @return gameplay event handle, which can be used to remove this receiver
	 */
	template <typename TEvent>
	FGameplayEventHandle AddReceiver(FGameplayTag Channel, const TEventCallbackWithTag<TEvent>& Callback)
	{
		const UScriptStruct* EventType = TBaseStructure<TEvent>::Get();
		FGameplayEventHandle EventHandle{this, GenerateNewID(), Channel, EventType};
		
		FWrapperCallback ThunkCallback = FWrapperCallback::CreateLambda([InnerCallback = Callback]
			(FGameplayTag Channel, const void* Event)
		{
			if (InnerCallback.IsBound())
			{
				InnerCallback.Execute(Channel, *static_cast<const TEvent*>(Event));
			}
		});
		
		EventHandle.DelegateHandle = AddReceiverInternal(Channel, EventType, MoveTemp(ThunkCallback));

		return EventHandle;
	}

	/**
	 * Remove previously registered event receiver
	 * @param EventHandle handle returned by @AddReceiver
	 */ 
	void RemoveReceiver(const FGameplayEventHandle& EventHandle)
	{
		RemoveReceiverInternal(EventHandle);
	}

#if 0
	void RemoveAll(const void* UserObject)
	{
		for (auto& [Struct, Container]: EventContainers)
		{
			Container->RemoveAll(UserObject);
		}
	}
#endif

protected:
	
	/**
	 * Send gameplay event on specified channel with payload.
	 * @param WorldContextObject
	 * @param Channel event channel
	 * @param Event event payload
	 * @param SendMode send mode, either immediate or async
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Events", meta = (DefaultToSelf = "WorldContextObject", CustomStructureParam = "Event", AllowAbstract = "false", DisplayName = "Send Event"))
	static void K2_SendEvent(const UObject* WorldContextObject, FGameplayTag Channel, const int32& Event, ESendEventMode SendMode);

	DECLARE_FUNCTION(execK2_SendEvent);

	/**
	 * Send event with given type on a given channel
	 * @param Channel event channel
	 * @param EventType event type
	 * @param Event event to send
	 * @param SendMode send mode, either immediate or async
	 */
	void SendEventInternal(const FGameplayTag& Channel, const UScriptStruct* EventType, const void* Event, ESendEventMode SendMode);

	/** add event receiver for given event type */
	FDelegateHandle AddReceiverInternal(const FGameplayTag& Channel, const UScriptStruct* EventType, FWrapperCallback&& InnerCallback);

	/** Remove event receiver using event handle */
	void RemoveReceiverInternal(const FGameplayEventHandle& EventHandle);
	
	/** @return gameplay event container for given event type */
	FGameplayEventContainerRef GetOrCreateEventContainer(const UScriptStruct* EventType);

	static uint32 GenerateNewID();

	TMap<const UStruct*, TSharedPtr<FGameplayEventContainer>> EventContainers;
	static uint32 HandleID;
};
