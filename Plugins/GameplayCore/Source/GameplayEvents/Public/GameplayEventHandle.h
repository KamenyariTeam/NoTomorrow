#pragma once

#include "GameplayTagContainer.h"

class UGameplayEventSubsystem;

struct GAMEPLAYEVENTS_API FGameplayEventHandle
{
	friend class UGameplayEventSubsystem;
	friend class UAsyncAction_WaitGameplayEvent;
public:

	FGameplayEventHandle() = default;

	bool IsValid() const;

	void Invalidate() const;

	friend FORCEINLINE bool operator==(const FGameplayEventHandle& Lhs, const FGameplayEventHandle& Rhs)
	{
		return Lhs.HandleID == Rhs.HandleID;
	}

	friend FORCEINLINE bool operator!=(const FGameplayEventHandle& Lhs, const FGameplayEventHandle& Rhs)
	{
		return !(Lhs == Rhs);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FGameplayEventHandle& Handle)
	{
		return HashCombine(GetTypeHash(Handle.Channel), GetTypeHash(Handle.DelegateHandle));
	}
	
private:

	FGameplayEventHandle(UGameplayEventSubsystem* InSubsystem, uint32 InID, const FGameplayTag& InChannel, const UScriptStruct* InEventType)
		: Subsystem(InSubsystem)
		, HandleID(InID)
		, Channel(InChannel)
		, EventType(InEventType)
	{ }
	
	mutable UGameplayEventSubsystem* Subsystem = nullptr;
	mutable uint32 HandleID = 0;
	
	FGameplayTag Channel;
	TWeakObjectPtr<const UScriptStruct> EventType;
	
	FDelegateHandle DelegateHandle;
};

