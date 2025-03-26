#include "GameplayExperienceTests.h"

#include "GameExperienceManagerComponent.h"
#include "GameplayExperience.h"
#include "Engine/AssetManager.h"


UTestGameplayExperience::UTestGameplayExperience()
{
	auto TestAction = CreateDefaultSubobject<UGameplayExperienceTestGameFeatureAction>(TEXT("TestGameFeatureAction0"));
	Actions.Add(TestAction);
}

AGameplayExperienceTestGameState::AGameplayExperienceTestGameState()
{
	ExperienceManager = CreateDefaultSubobject<UGameExperienceManagerComponent>(TEXT("Experience Manager"));
}

class FGameplayExperienceWorld
{
public:
	FGameplayExperienceWorld()
	{
		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();
		
		World = GameInstance->GetWorld();

		FURL URL;
		World->SetGameMode(URL);
		World->InitializeActorsForPlay(URL);
		World->BeginPlay();

		InitialFrameCounter = GFrameCounter;
	}

	~FGameplayExperienceWorld()
	{
		GFrameCounter = InitialFrameCounter;

		GameInstance->Shutdown();
		
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);

		GameInstance->MarkAsGarbage();
	}

	UWorld* GetWorld() const
	{
		return World;
	}

	void TickWorld(float Time)
	{
		const float DeltaTime = 1.0 / 60.0; // tick at 60 frames
		while (Time > 0.f)
		{
			World->Tick(ELevelTick::LEVELTICK_All, FMath::Min(Time, DeltaTime));
			// tick streamable manager
			FTickableGameObject::TickObjects(nullptr, LEVELTICK_All, false, DeltaTime);
			Time -= DeltaTime;
			
			++GFrameCounter;
		}
	}

private:
	UWorld* World;
	UGameInstance* GameInstance;
	
	uint64 InitialFrameCounter = 0;
};

class FGameplayExperienceAutomationSuite
{
public:
	FGameplayExperienceAutomationSuite(FGameplayExperienceWorld& InWorld, FAutomationTestBase* InTest)
		: World(InWorld)
		, Test(InTest)
	{
		FPrimaryAssetId TestExperienceID{FPrimaryAssetType(UTestGameplayExperience::StaticClass()->GetFName()), TEXT("TestExperience")};
		FSoftObjectPath ExperiencePath = UAssetManager::Get().GetPrimaryAssetPath(TestExperienceID);

		GameplayExperience = CastChecked<UTestGameplayExperience>(ExperiencePath.TryLoad());
		TestFeatureAction = CastChecked<UGameplayExperienceTestGameFeatureAction>(GameplayExperience->Actions[0]);
		
		FActorSpawnParameters Parameters;
		Parameters.bDeferConstruction = true;
		Parameters.ObjectFlags |= RF_Transient;
		GameState = World.GetWorld()->SpawnActor<AGameplayExperienceTestGameState>(AGameplayExperienceTestGameState::StaticClass(), Parameters);
		GameState->ExperienceID = GameplayExperience->GetPrimaryAssetId();
		GameState->FinishSpawning(FTransform::Identity);
	}

	void RunTest()
	{
		const auto Manager = GameState->ExperienceManager;
		Test->TestTrue(TEXT("IsGameExperienceLoaded() || IsGameExperienceLoading()"), Manager->IsGameExperienceLoaded() || Manager->IsGameExperienceLoading());

		while (Manager->IsGameExperienceLoading())
		{
			World.TickWorld(1.f);
		}

		Test->TestTrue(TEXT("IsGameExperienceLoaded() == true"), Manager->IsGameExperienceLoaded() == true);
		Test->TestTrue(TEXT("GetGameExperience() == TestExperience"), Manager->GetGameExperience() == GameplayExperience);
		Test->TestTrue(TEXT("TestFeatureAction->IsRegistered() == true"), TestFeatureAction->IsRegistered() == true);
		Test->TestTrue(TEXT("TestFeatureAction->IsActivated() == true"), TestFeatureAction->IsActivated() == true);

		Manager->DestroyComponent();

		Test->TestTrue(TEXT("IsGameExperienceLoaded() == false"), Manager->IsGameExperienceLoaded() == false);
		Test->TestTrue(TEXT("GetGameExperience() == nullptr"), Manager->GetGameExperience() == nullptr);
		Test->TestTrue(TEXT("TestFeatureAction->IsRegistered() == false"), TestFeatureAction->IsRegistered() == false);
		Test->TestTrue(TEXT("TestFeatureAction->IsActivated() == false"), TestFeatureAction->IsActivated() == false);
	}

	~FGameplayExperienceAutomationSuite()
	{
		GameplayExperience->MarkAsGarbage();
		TestFeatureAction->MarkAsGarbage();
	}

private:
	FGameplayExperienceWorld& World;
	FAutomationTestBase* Test;

	AGameplayExperienceTestGameState* GameState = nullptr;
	UGameplayExperience* GameplayExperience = nullptr;
	UGameplayExperienceTestGameFeatureAction* TestFeatureAction = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayExperienceAutomationTest, "System.GameplayExperience.GameLoop", EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask)

bool FGameplayExperienceAutomationTest::RunTest(const FString& Parameters)
{
	FGameplayExperienceWorld World;

	FGameplayExperienceAutomationSuite TestSuite(World, this);

	TestSuite.RunTest();

	return !HasAnyErrors();
}
