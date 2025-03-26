#include "GameplayEventSubsystem.h"

#include "GameplayEvents.h"
#include "GameplayTagsManager.h"

static bool GShouldLogGameplayEvents = false;
static FAutoConsoleVariableRef CVarShouldLogGameplayEvents(
	TEXT("GameplayEvents.LogEvents"),
	GShouldLogGameplayEvents,
	TEXT("Whether events sent through gameplay events subsystem should logged")
);

static bool GAllowNonLeafEventChannels = false;
static FAutoConsoleVariableRef CVarAllowNonLeafEventChannels(
	TEXT("GameplayEvents.AllowNonLeafEventChannels"),
	GAllowNonLeafEventChannels,
	TEXT("Whether non-leaf tag channels should be allowed for SendEvent")
);

static bool GShouldDumpCallstack = true;
static FAutoConsoleVariableRef CVarShouldDumpCallstack(
	TEXT("GameplayEvents.ShouldDumpCallstack"),
	GShouldDumpCallstack,
	TEXT("Whether dump callstack for invalid channel tags")
);

uint32 UGameplayEventSubsystem::HandleID = 1;

UGameplayEventSubsystem* UGameplayEventSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	check(World);
	
	return UGameInstance::GetSubsystem<UGameplayEventSubsystem>(World->GetGameInstance());
}

void UGameplayEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EventContainers.Reset();
}

void UGameplayEventSubsystem::Deinitialize()
{
	EventContainers.Reset();
	
	Super::Deinitialize();
}

void UGameplayEventSubsystem::Tick(float DeltaTime)
{
	for (auto& [Struct, Container]: EventContainers)
	{
		Container->BroadcastAsync();
	}
}

void UGameplayEventSubsystem::K2_SendEvent(const UObject* WorldContextObject, FGameplayTag Channel, const int32& Event, ESendEventMode SendEventMode)
{
	// Exec version should be hit instead
	checkNoEntry();	
}

DEFINE_FUNCTION(UGameplayEventSubsystem::execK2_SendEvent)
{
	P_GET_OBJECT(const UObject, WorldContextObject);
	P_GET_STRUCT(FGameplayTag, Channel);
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const void* EventPtr = Stack.MostRecentPropertyAddress;
	const FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_ENUM(ESendEventMode, SendEventMode);

	P_FINISH;

	if (ensure((StructProperty != nullptr) && (StructProperty->Struct != nullptr) && (EventPtr != nullptr)))
	{
		if (UGameplayEventSubsystem* EventSubsystem = UGameplayEventSubsystem::Get(WorldContextObject))
		{
			EventSubsystem->SendEventInternal(Channel, StructProperty->Struct, EventPtr, SendEventMode);
		}
	}
}

void UGameplayEventSubsystem::SendEventInternal(const FGameplayTag& Channel, const UScriptStruct* EventType, const void* Event, ESendEventMode SendMode)
{
	if (!Channel.IsValid())
	{
		return;
	}
	
#if !UE_BUILD_SHIPPING
	if (!GAllowNonLeafEventChannels)
	{
		const FGameplayTagContainer LeafTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(Channel);
		if (!LeafTags.IsEmpty())
		{
			UE_LOG(LogGameplayEvents, Error, TEXT("Broadcasting non-leaf tags is disabled. Possible channels: [%s]. Actual channel: [%s]"), *LeafTags.ToString(), *Channel.ToString());
			if (GShouldDumpCallstack)
			{
				const uint32 DumpCallstackSize = 65535;
				ANSICHAR DumpCallstack[DumpCallstackSize] = { 0 };
				const FString ScriptStack = FFrame::GetScriptCallstack(true);
				FPlatformStackWalk::StackWalkAndDump(DumpCallstack, DumpCallstackSize, 0);

				UE_LOG(LogGameplayEvents, Error, TEXT("--- Callstack ---"));
				UE_LOG(LogGameplayEvents, Error, TEXT("Script Stack:\n%s"), *ScriptStack);
				UE_LOG(LogGameplayEvents, Error, TEXT("Callstack:\n%s"), ANSI_TO_TCHAR(DumpCallstack));
			}
		}
	}
	
	if (GShouldLogGameplayEvents)
	{
		FString ContextString = GetPathNameSafe(this);
#if WITH_EDITOR
		if (GIsEditor)
		{
			// forward declare GPlayInEditorContextString from UnrealEngine.cpp
			extern ENGINE_API FString GPlayInEditorContextString;
			ContextString = GPlayInEditorContextString;
		}
#endif

		const FString SendModeString = SendMode == ESendEventMode::Immediate ? TEXT("Immediate") : TEXT("Async)");
		
		FString EventString;
		EventType->ExportText(EventString, Event, nullptr, nullptr, PPF_None, nullptr);
		
		UE_LOG(LogGameplayEvents, Display, TEXT("Gameplay Event: Context: [%s], Channel: [%s], Event: [%s], Mode: [%s]"),
			*ContextString, *Channel.ToString(), *EventString, *SendModeString);
	}
#endif
	
	FGameplayEventContainerRef EventContainer = GetOrCreateEventContainer(EventType);

	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		FGameplayEventContainer::FChannelEvent Data{Channel, Tag, Event};
	
		EventContainer->Broadcast(MoveTemp(Data), SendMode);
	}

}

FDelegateHandle UGameplayEventSubsystem::AddReceiverInternal(const FGameplayTag& Channel, const UScriptStruct* EventType, FWrapperCallback&& InnerCallback)
{
	FGameplayEventContainerRef EventContainer = GetOrCreateEventContainer(EventType);
	return EventContainer->Add(Channel, MoveTemp(InnerCallback));
}

void UGameplayEventSubsystem::RemoveReceiverInternal(const FGameplayEventHandle& EventHandle)
{
	if (!EventHandle.IsValid())
	{
		return;
	}
	
	FGameplayEventContainerRef EventContainer = GetOrCreateEventContainer(EventHandle.EventType.Get());
	EventContainer->Remove(EventHandle);

	EventHandle.Invalidate();
}

UGameplayEventSubsystem::FGameplayEventContainerRef UGameplayEventSubsystem::GetOrCreateEventContainer(const UScriptStruct* EventType)
{
	FGameplayEventContainerPtr Container;
	if (TSharedPtr<FGameplayEventContainer>* ContainerPtr = EventContainers.Find(EventType))
	{
		Container = StaticCastSharedPtr<FGameplayEventContainer>(*ContainerPtr);
	}
	else
	{
		Container = MakeShared<FGameplayEventContainer>();
		EventContainers.Add(EventType, Container);
	}

	return Container.ToSharedRef();
}

uint32 UGameplayEventSubsystem::GenerateNewID()
{
	uint32 Result = ++HandleID;

	if (UNLIKELY(Result == 0))
	{
		Result = ++HandleID;
	}

	return Result;
}
