// © 2025 Kamenyari. All rights reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/NotoCombatDummy.h"

#include "AbilitySystemComponent.h"
#include "Combat/NotoDamageEffect.h"
#include "Combat/NotoHealthComponent.h"
#include "Combat/NotoHealthSet.h"
#include "Development/NotoGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

namespace NotoCombatTests
{
	struct FTestWorld
	{
		FTestWorld()
		{
			World = NewObject<UWorld>(GetTransientPackage());
			World->WorldType = EWorldType::GamePreview;
			FWorldContext& WorldContext = GEngine->CreateNewWorldContext(World->WorldType);
			WorldContext.SetCurrentWorld(World);
			World->InitializeNewWorld(UWorld::InitializationValues()
			                          .AllowAudioPlayback(false)
			                          .CreatePhysicsScene(false)
			                          .CreateNavigation(false)
			                          .CreateAISystem(false)
			                          .ShouldSimulatePhysics(false)
			                          .SetTransactional(false)
			                          .CreateFXSystem(false));
			World->InitializeActorsForPlay(FURL());
		}

		~FTestWorld()
		{
			World->CleanupWorld();
			GEngine->DestroyWorldContext(World);
			World->ReleasePhysicsScene();
		}

		UWorld* World = nullptr;
	};

	bool ApplyDamage(
		ANotoCombatDummy& Source,
		ANotoCombatDummy& Target,
		float Damage,
		UObject* SourceObject)
	{
		UAbilitySystemComponent* SourceAbilitySystem = Source.GetAbilitySystemComponent();
		UAbilitySystemComponent* TargetAbilitySystem = Target.GetAbilitySystemComponent();
		if (!SourceAbilitySystem || !TargetAbilitySystem)
		{
			return false;
		}

		FGameplayEffectContextHandle Context = SourceAbilitySystem->MakeEffectContext();
		Context.AddInstigator(&Source, &Source);
		Context.AddSourceObject(SourceObject);
		FHitResult HitResult;
		HitResult.bBlockingHit = true;
		HitResult.ImpactPoint = FVector(100.0f, 0.0f, 50.0f);
		Context.AddHitResult(HitResult, true);
		FGameplayEffectSpecHandle Spec = SourceAbilitySystem->MakeOutgoingSpec(
			UNotoDamageEffect::StaticClass(),
			1.0f,
			Context);
		if (!Spec.IsValid())
		{
			return false;
		}

		Spec.Data->SetSetByCallerMagnitude(NotoGameplayTags::Data_Damage, Damage);
		SourceAbilitySystem->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetAbilitySystem);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoGasDamageAndDeathTest,
	"NoTomorrow.Combat.GAS.DamageAndDeath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoGasDamageAndDeathTest::RunTest(const FString& Parameters)
{
	using namespace NotoCombatTests;

	FTestWorld TestWorld;
	ANotoCombatDummy* Source = TestWorld.World->SpawnActor<ANotoCombatDummy>();
	ANotoCombatDummy* Target = TestWorld.World->SpawnActor<ANotoCombatDummy>();
	TestNotNull(TEXT("Source dummy spawned"), Source);
	TestNotNull(TEXT("Target dummy spawned"), Target);
	if (!Source || !Target)
	{
		return false;
	}
	Source->GetAbilitySystemComponent()->InitAbilityActorInfo(Source, Source);
	Target->GetAbilitySystemComponent()->InitAbilityActorInfo(Target, Target);
	Source->GetHealthComponent()->InitializeWithAbilitySystem(Source->GetAbilitySystemComponent());
	Target->GetHealthComponent()->InitializeWithAbilitySystem(Target->GetAbilitySystemComponent());

	UNotoHealthSet* TargetHealthSet = Target->GetHealthSet();
	UNotoHealthComponent* TargetHealthComponent = Target->GetHealthComponent();
	TestNotNull(TEXT("Target owns health attributes"), TargetHealthSet);
	TestNotNull(TEXT("Target owns a health component"), TargetHealthComponent);
	if (!TargetHealthSet || !TargetHealthComponent)
	{
		return false;
	}

	FNotoDamageEvent LastDamageEvent;
	const FDelegateHandle DamageHandle = TargetHealthSet->OnDamageReceived().AddLambda(
		[&LastDamageEvent](const FNotoDamageEvent& DamageEvent)
		{
			LastDamageEvent = DamageEvent;
		});
	UObject* WeaponSource = NewObject<UNotoDamageEffect>(GetTransientPackage());

	TestTrue(TEXT("Damage effect can be applied"), ApplyDamage(*Source, *Target, 25.0f, WeaponSource));
	TestEqual(TEXT("Damage reduces health"), TargetHealthSet->GetHealth(), 75.0f);
	TestEqual(TEXT("Damage event records actual damage"), LastDamageEvent.Damage, 25.0f);
	TestEqual(TEXT("Damage event records the target avatar"), LastDamageEvent.Target.Get(), static_cast<AActor*>(Target));
	TestEqual(TEXT("Damage event records the instigator"), LastDamageEvent.Instigator.Get(), static_cast<AActor*>(Source));
	TestEqual(TEXT("Damage event records the causer"), LastDamageEvent.DamageCauser.Get(), static_cast<AActor*>(Source));
	TestEqual(TEXT("Damage event records the weapon source"), LastDamageEvent.SourceObject.Get(), WeaponSource);
	TestTrue(TEXT("Damage event records a blocking hit"), LastDamageEvent.HitResult.bBlockingHit);
	TestEqual(TEXT("Damage event records the impact point"), LastDamageEvent.HitResult.ImpactPoint,
	          FVector(100.0f, 0.0f, 50.0f));
	TestFalse(TEXT("Surviving target is alive"), TargetHealthComponent->IsDead());

	TestTrue(TEXT("Lethal damage effect can be applied"), ApplyDamage(*Source, *Target, 100.0f, WeaponSource));
	TestEqual(TEXT("Health clamps at zero"), TargetHealthSet->GetHealth(), 0.0f);
	TestEqual(TEXT("Overkill reports only remaining health"), LastDamageEvent.Damage, 75.0f);
	TestTrue(TEXT("Health component enters death state"), TargetHealthComponent->IsDead());
	TestTrue(
		TEXT("Ability system owns the dead gameplay tag"),
		Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(NotoGameplayTags::State_Dead));

	TargetHealthSet->OnDamageReceived().Remove(DamageHandle);
	return true;
}

#endif
