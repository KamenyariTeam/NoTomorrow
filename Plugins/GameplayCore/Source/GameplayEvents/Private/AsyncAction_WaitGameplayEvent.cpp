#include "AsyncAction_WaitGameplayEvent.h"

#include "GameplayEventSubsystem.h"

UAsyncAction_WaitGameplayEvent* UAsyncAction_WaitGameplayEvent::WaitForGameplayEvent(const UObject* WorldContextObject, FGameplayTag Channel, const UScriptStruct* EventType)
{
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	UAsyncAction_WaitGameplayEvent* Action = NewObject<UAsyncAction_WaitGameplayEvent>();
	Action->World = World;
	Action->Channel = Channel;
	Action->EventType = EventType;
	Action->RegisterWithGameInstance(World->GetGameInstance());

	return Action;
}

void UAsyncAction_WaitGameplayEvent::Activate()
{
	Super::Activate();

	if (UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(World.Get()))
	{
		EventHandle = FGameplayEventHandle{Subsystem, Subsystem->GenerateNewID(), Channel, EventType.Get()};
		
		TWeakObjectPtr<ThisClass> WeakThis(this);
		auto WrapperCallback = UGameplayEventSubsystem::FWrapperCallback::CreateLambda([WeakThis](FGameplayTag ActualTag, const void* Event)
		{
			if (UAsyncAction_WaitGameplayEvent* This = WeakThis.Get())
			{
				This->HandleEventReceived(ActualTag, Event);
			}
		});
		EventHandle.DelegateHandle = Subsystem->AddReceiverInternal(Channel, EventType.Get(), MoveTemp(WrapperCallback));
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_WaitGameplayEvent::SetReadyToDestroy()
{
	if (World.IsValid())
	{
		if (UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(World.Get()))
		{
			Subsystem->RemoveReceiver(EventHandle);
		}
	}
	
	Super::SetReadyToDestroy();
}

bool UAsyncAction_WaitGameplayEvent::GetEventData(int32& OutEventData)
{
	// Exec version should be hit instead
	checkNoEntry();
	return false;
}

DEFINE_FUNCTION(UAsyncAction_WaitGameplayEvent::execGetEventData)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* EventPtr = Stack.MostRecentPropertyAddress;
	const FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;

	// Make sure the type we are trying to get through the blueprint node matches the type of the message payload received.
	if ((StructProperty != nullptr) && (StructProperty->Struct != nullptr) && (EventPtr != nullptr) && (StructProperty->Struct == P_THIS->EventType.Get()) && (P_THIS->ReceivedEventData != nullptr))
	{
		StructProperty->Struct->CopyScriptStruct(EventPtr, P_THIS->ReceivedEventData);
		bSuccess = true;
	}

	*(bool*)RESULT_PARAM = bSuccess;
}

void UAsyncAction_WaitGameplayEvent::HandleEventReceived(FGameplayTag ActualChannel, const void* Event)
{
	ReceivedEventData = Event;
	Received.Broadcast(this, ActualChannel);
	ReceivedEventData = nullptr;
	
	if (!Received.IsBound())
	{
		// If the BP object that created the async node is destroyed, Received will be unbound after calling the broadcast.
		// In this case we can safely mark this receiver as ready for destruction.
		SetReadyToDestroy();
	}
}
