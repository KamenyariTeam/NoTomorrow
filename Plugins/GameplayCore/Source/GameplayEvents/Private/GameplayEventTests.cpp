
#include "GameplayEventSubsystem.h"
#include "GameplayTagsManager.h"

constexpr EAutomationTestFlags AutomationFlags = EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask;

FGameplayTag CreateChannel(const FName& Name)
{
	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(Name);
	return Channel;
#if 0
	FGameplayTag Channel = FGameplayTag::EmptyTag;
	
	FProperty* Property = TBaseStructure<FGameplayTag>::Get()->FindPropertyByName(TEXT("TagName"));
	if (const FNameProperty* NameProperty = CastFieldChecked<FNameProperty>(Property))
	{
		NameProperty->SetValue_InContainer(&Channel, Name);
	}
	check(Channel.IsValid());

	return Channel;
#endif
}

UGameplayEventSubsystem* CreateSubsystem()
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	UGameplayEventSubsystem* Subsystem = NewObject<UGameplayEventSubsystem>(GameInstance, NAME_None, RF_Transient);

	return Subsystem;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventSubsystemTest_Handle, "Game.GameplayEvents.Handle", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventSubsystemTest_Handle::RunTest(const FString& Parameters)
{
	UGameplayEventSubsystem* Subsystem = CreateSubsystem();
	const FGameplayTag Channel = CreateChannel(TEXT("Tests.Channel"));

	{
		FGameplayEventHandle EmptyHandle{};
		TestTrueExpr(!EmptyHandle.IsValid());
	}
	
	const auto Callback = TEventCallbackWithTag<FVector>::CreateLambda([](FGameplayTag, const FVector&) { });
	{
		FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, Callback);

		TestTrueExpr(Handle.IsValid());
		Handle.Invalidate();
		TestTrueExpr(!Handle.IsValid());
	}

	{
		FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, Callback);
		TestTrueExpr(Handle.IsValid());
		Subsystem->RemoveReceiver(Handle);
		TestTrueExpr(!Handle.IsValid());
	}

	{
		const FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, Callback);
		TestTrueExpr(Handle.IsValid());
		Subsystem->RemoveReceiver(Handle);
		TestTrueExpr(!Handle.IsValid());
	}

	{
		FGameplayEventHandle EmptyHandle1{}, EmptyHandle2{};
		FGameplayEventHandle Handle1 = Subsystem->AddReceiver(Channel, Callback);
		FGameplayEventHandle Handle2 = Subsystem->AddReceiver(Channel, Callback);

		TestTrueExpr(Handle1.IsValid() && Handle2.IsValid());
		TestTrueExpr(Handle1 != Handle2);
		TestTrueExpr(EmptyHandle1 == EmptyHandle2);
		TestTrueExpr(Handle1 != EmptyHandle1);
	}

	{
		FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, Callback);
		TestTrueExpr(Handle.IsValid());

		Subsystem->RemoveReceiver(Handle);
		TestTrueExpr(!Handle.IsValid());
		
		Subsystem->RemoveReceiver(Handle);
		TestTrueExpr(!Handle.IsValid());

		Subsystem->RemoveReceiver(FGameplayEventHandle{});
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventTest_Callbacks, "Game.GameplayEvents.Callbacks", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventTest_Callbacks::RunTest(const FString& Parameters)
{
	struct FThunk
	{
		void CallbackWithTag(FGameplayTag Channel, const FVector& InValue)
		{
			++CallbackCount;
			Value = InValue;
		}

		void Callback(const FVector& InValue)
		{
			++CallbackCount;
			Value = InValue;
		}

		int32 CallbackCount = 0;
		FVector Value = FVector::ZeroVector;
	};

	FThunk Thunk;
	auto& [CallbackCount, Value] = Thunk;
	const auto Callback = TEventCallback<FVector>::CreateRaw(&Thunk, &FThunk::Callback);
	const auto CallbackWithTag = TEventCallbackWithTag<FVector>::CreateRaw(&Thunk, &FThunk::CallbackWithTag);

	const FGameplayTag Channel = CreateChannel(TEXT("Tests.Channel.Gameplay"));

	UGameplayEventSubsystem* Subsystem = CreateSubsystem();
	{
		FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, Callback);
		
		// broadcast works
		const FVector V1{1, 1, 1};
		Subsystem->SendEvent(Channel, V1);

		UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 1);
		UTEST_EQUAL(TEXT("CallbackValue"), Value, (V1));

		Subsystem->RemoveReceiver(Handle);
	}

	{
		FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, CallbackWithTag);

		// broadcast works
		const FVector V2{2, 2, 2};
		Subsystem->SendEvent(Channel, V2);

		UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 2);
		UTEST_EQUAL(TEXT("CallbackValue"), Value, (V2));

		Subsystem->RemoveReceiver(Handle);
	}

	{
		Subsystem->AddReceiver(Channel, Callback);
		Subsystem->AddReceiver(Channel, CallbackWithTag);

		// broadcast works
		const FVector V3{3, 3, 3};
		Subsystem->SendEvent(Channel, V3);

		UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 4);
		UTEST_EQUAL(TEXT("CallbackValue"), Value, (V3));
	}

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventTest_Events, "Game.GameplayEvents.Events", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventTest_Events::RunTest(const FString& Parameters)
{
	struct FThunk
	{
		void Callback(FGameplayTag Channel, const FVector& InValue)
		{
			++CallbackCount;
			Value = InValue;
		}

		int32 CallbackCount = 0;
		FVector Value = FVector::ZeroVector;
	};

	FThunk Thunk;
	auto& [CallbackCount, Value] = Thunk;
	const auto Callback = TEventCallbackWithTag<FVector>::CreateRaw(&Thunk, &FThunk::Callback);

	UGameplayEventSubsystem* Subsystem = CreateSubsystem();
	const FGameplayTag Channel = CreateChannel(TEXT("Tests.Channel.Gameplay"));
	
	FGameplayEventHandle Handle = Subsystem->AddReceiver(Channel, Callback);

	// broadcast works
	const FVector V1{1, 1, 1};
	Subsystem->SendEvent(Channel, V1);
		
	UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 1);
	UTEST_EQUAL(TEXT("CallbackValue"), Value, (V1));

	//broadcast async works
	FVector V2{2, 2, 2};
	Subsystem->SendEventAsync(Channel, V2);

	UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 1);
	UTEST_EQUAL(TEXT("CallbackValue"), Value, (V1));

	Subsystem->Tick(1.0 / 60);

	UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 2);
	UTEST_EQUAL(TEXT("CallbackValue"), Value, (V2));

	// removing by handle works
	Subsystem->RemoveReceiver(Handle);
	Subsystem->SendEvent(Channel, FVector{3, 3, 3});

	UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 2); 
	UTEST_EQUAL(TEXT("CallbackValue"), Value, (V2));

	// removing by source works
	Subsystem->AddReceiver(Channel, Callback);
	Subsystem->AddReceiver(Channel, Callback);

	FVector V4{4, 4, 4};
	Subsystem->SendEvent(Channel, V4);
	
	UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 4); 
	UTEST_EQUAL(TEXT("CallbackValue"), Value, (V4));

#if 0
	Subsystem->RemoveAll(&Thunk);
	Subsystem->SendEvent(Channel, FVector::ZeroVector);

	UTEST_EQUAL(TEXT("CallbackCount"), CallbackCount, 4);
	UTEST_EQUAL(TEXT("CallbackValue"), Value, (V4));
#endif
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventTest_Channels, "Game.GameplayEvents.Channels", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventTest_Channels::RunTest(const FString& Parameters)
{
	UGameplayEventSubsystem* Subsystem = CreateSubsystem();
	
	const FGameplayTag Gameplay = CreateChannel(TEXT("Tests.Channel.Gameplay"));
	const FGameplayTag UI		= CreateChannel(TEXT("Tests.Channel.UI"));

	int32 GameplayCount = 0;
	int32 UICount = 0;
	const FVector Value = FVector::ZeroVector;
	
	auto GameplayCallback = TEventCallbackWithTag<FVector>::CreateLambda([&GameplayCount](FGameplayTag, FVector){ ++GameplayCount; });
	auto UICallback = TEventCallbackWithTag<FVector>::CreateLambda([&UICount](FGameplayTag, FVector) { ++UICount; });
	
	Subsystem->AddReceiver(Gameplay, GameplayCallback);
	Subsystem->SendEvent(Gameplay, Value);

	UTEST_EQUAL(TEXT("GameplayCount"), GameplayCount, 1);
	UTEST_EQUAL(TEXT("UICount"), UICount, 0);

	Subsystem->AddReceiver(UI, UICallback);
	Subsystem->SendEvent(UI, Value);

	UTEST_EQUAL(TEXT("GameplayCount"), GameplayCount, 1);
	UTEST_EQUAL(TEXT("UICount"), UICount, 1);

	Subsystem->AddReceiver(Gameplay, GameplayCallback);
	Subsystem->SendEvent(Gameplay, Value);
	
	UTEST_EQUAL(TEXT("GameplayCount"), GameplayCount, 3);
	UTEST_EQUAL(TEXT("UICount"), UICount, 1);
	
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventTest_SubChannels, "Game.GameplayEvents.SubChannels", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventTest_SubChannels::RunTest(const FString& Parameters)
{
	// disable non-leaf event channels to test them
	IConsoleVariable* AllowNonLeafEventChannels = IConsoleManager::Get().FindConsoleVariable(TEXT("GameplayEvents.AllowNonLeafEventChannels"));
	bool bAllowNonLeafEventChannels = AllowNonLeafEventChannels->GetBool();
	AllowNonLeafEventChannels->Set(false);

	IConsoleVariable* DumpCallstack = IConsoleManager::Get().FindConsoleVariable(TEXT("GameplayEvents.ShouldDumpCallstack"));
	bool bDumpCallstack = DumpCallstack->GetBool();
	DumpCallstack->Set(false);

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	UGameplayEventSubsystem* Subsystem = NewObject<UGameplayEventSubsystem>(GameInstance, NAME_None, RF_Transient);
	
	const FGameplayTag Parent = CreateChannel(TEXT("Tests.Channel.Parent"));
	const FGameplayTag Child = CreateChannel(TEXT("Tests.Channel.Parent.Child"));
	const FGameplayTag Leaf = CreateChannel(TEXT("Tests.Channel.Parent.Child.Leaf"));

	AddExpectedError(TEXT("Broadcasting non-leaf tags is disabled."), EAutomationExpectedErrorFlags::Contains, 2);

	// should produce an expected error
	Subsystem->SendEvent(Parent, FVector::ZeroVector);
	// should produce an expected error
	Subsystem->SendEvent(Child, FVector::ZeroVector);
	// fine
	Subsystem->SendEvent(Leaf, FVector::ZeroVector);

	AllowNonLeafEventChannels->Set(bAllowNonLeafEventChannels);
	DumpCallstack->Set(bDumpCallstack);
	
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventTest_EventLogging, "Game.GameplayEvents.EventLogging", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventTest_EventLogging::RunTest(const FString& Parameters)
{
	// enable gameplay event logging
	IConsoleVariable* LogEvents = IConsoleManager::Get().FindConsoleVariable(TEXT("GameplayEvents.LogEvents"));
	bool bLogEvents = LogEvents->GetBool();
	LogEvents->Set(true);

	UGameplayEventSubsystem* Subsystem = CreateSubsystem();
	const FGameplayTag Gameplay = CreateChannel(TEXT("Tests.Channel.Gameplay"));
	const FGameplayTag UI = CreateChannel(TEXT("Tests.Channel.UI"));
	
	AddExpectedMessage(TEXT("Tests\\.Channel\\.Gameplay.+Immediate"), ELogVerbosity::Display, EAutomationExpectedMessageFlags::Contains, 1);
	Subsystem->SendEvent(Gameplay, FVector::ZeroVector);
	
	AddExpectedMessage(TEXT("Tests\\.Channel\\.UI.+Async"), ELogVerbosity::Display, EAutomationExpectedMessageFlags::Contains, 1);
	Subsystem->SendEventAsync(UI, FVector::ZeroVector);
	
	LogEvents->Set(bLogEvents);
	
	return !HasAnyErrors();
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEventTest_ActualChannels, "Game.GameplayEvents.ActualChannels", AutomationFlags | EAutomationTestFlags::MediumPriority)

bool FGameplayEventTest_ActualChannels::RunTest(const FString& Parameters)
{
	UGameplayEventSubsystem* Subsystem = CreateSubsystem();
	
	const FGameplayTag Parent = CreateChannel(TEXT("Tests.Channel.Parent"));
	const FGameplayTag Child = CreateChannel(TEXT("Tests.Channel.Parent.Child"));
	const FGameplayTag Leaf = CreateChannel(TEXT("Tests.Channel.Parent.Child.Leaf"));

	int32 Count = 0;
	FVector Value = FVector::ZeroVector;
	FGameplayTag Channel = FGameplayTag::EmptyTag;

	// allow non-leaf event channels to test them
	IConsoleVariable* AllowNonLeafEventChannels = IConsoleManager::Get().FindConsoleVariable(TEXT("GameplayEvents.AllowNonLeafEventChannels"));
	bool bAllowNonLeafEventChannels = AllowNonLeafEventChannels->GetBool();
	AllowNonLeafEventChannels->Set(true);
	
	const auto Callback = TEventCallbackWithTag<FVector>::CreateLambda([&](FGameplayTag InChannel, const FVector& InValue)
	{
		Channel = InChannel; Value = InValue; ++Count;
	});

	Count = 0;
	Value = FVector::ZeroVector;
	Channel = FGameplayTag::EmptyTag;
		
	FGameplayEventHandle Handle = Subsystem->AddReceiver(Parent, Callback);

	{
		Subsystem->SendEvent(Parent, FVector{1, 1, 1});
		UTEST_TRUE(TEXT("Count"), Count == 1);
		UTEST_TRUE(TEXT("Value"), (Value == FVector{1, 1, 1}));
		UTEST_TRUE(TEXT("Channel"), (Channel.MatchesTagExact(Parent)));
	}

	{
		Subsystem->SendEvent(Child, FVector{2, 2, 2});
		UTEST_TRUE(TEXT("Count"), Count == 2);
		UTEST_TRUE(TEXT("Value"), (Value == FVector{2, 2, 2}));
		UTEST_TRUE(TEXT("Channel"), (Channel.MatchesTagExact(Child)));
	}

	{
		Subsystem->SendEvent(Leaf, FVector{3, 3, 3});
		UTEST_TRUE(TEXT("Count"), Count == 3);
		UTEST_TRUE(TEXT("Value"), (Value == FVector{3, 3, 3}));
		UTEST_TRUE(TEXT("Channel"), (Channel.MatchesTagExact(Leaf)));
	}
	Subsystem->RemoveReceiver(Handle);

	Count = 0;
	Value = FVector::ZeroVector;
	Channel = FGameplayTag::EmptyTag;

	Handle = Subsystem->AddReceiver(Child, Callback);
	{
		Subsystem->SendEvent(Parent, FVector{1, 1, 1});
		UTEST_TRUE(TEXT("Count"), Count == 0);
		UTEST_TRUE(TEXT("Value"), (Value == FVector::ZeroVector));
		UTEST_TRUE(TEXT("Channel"), (Channel == FGameplayTag::EmptyTag));
	}

	{
		Subsystem->SendEvent(Child, FVector{2, 2, 2});
		UTEST_TRUE(TEXT("Count"), Count == 1);
		UTEST_TRUE(TEXT("Value"), (Value == FVector{2, 2, 2}));
		UTEST_TRUE(TEXT("Channel"), (Channel.MatchesTagExact(Child)));
	}

	{
		Subsystem->SendEvent(Leaf, FVector{3, 3, 3});
		UTEST_TRUE(TEXT("Count"), Count == 2);
		UTEST_TRUE(TEXT("Value"), (Value == FVector{3, 3, 3}));
		UTEST_TRUE(TEXT("Channel"), (Channel.MatchesTagExact(Leaf)));
	}
	Subsystem->RemoveReceiver(Handle);

	Count = 0;
	Value = FVector::ZeroVector;
	Channel = FGameplayTag::EmptyTag;

	Handle = Subsystem->AddReceiver(Leaf, Callback);
	{
		Subsystem->SendEvent(Parent, FVector{1, 1, 1});
		UTEST_TRUE(TEXT("Count"), Count == 0);
		UTEST_TRUE(TEXT("Value"), (Value == FVector::ZeroVector));
		UTEST_TRUE(TEXT("Channel"), (Channel == FGameplayTag::EmptyTag));
	}

	{
		Subsystem->SendEvent(Child, FVector{2, 2, 2});
		UTEST_TRUE(TEXT("Count"), Count == 0);
		UTEST_TRUE(TEXT("Value"), (Value == FVector::ZeroVector));
		UTEST_TRUE(TEXT("Channel"), (Channel == FGameplayTag::EmptyTag));
	}

	{
		Subsystem->SendEvent(Leaf, FVector{3, 3, 3});
		UTEST_TRUE(TEXT("Count"), Count == 1);
		UTEST_TRUE(TEXT("Value"), (Value == FVector{3, 3, 3}));
		UTEST_TRUE(TEXT("Channel"), (Channel.MatchesTagExact(Leaf)));
	}
	Subsystem->RemoveReceiver(Handle);

	AllowNonLeafEventChannels->Set(bAllowNonLeafEventChannels);

	return !HasAnyErrors();
}






