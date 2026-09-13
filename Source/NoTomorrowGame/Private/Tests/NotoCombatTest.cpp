// © 2025 Kamenyari. All rights reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/NotoCombatDummy.h"

#include "AbilitySystemComponent.h"
#include "Character/NotoCharacter.h"
#include "Combat/NotoDamageEffect.h"
#include "Combat/NotoFirearmComponent.h"
#include "Combat/NotoHealthComponent.h"
#include "Combat/NotoHealthSet.h"
#include "Development/NotoGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Inventory/NotoInventoryComponent.h"
#include "Inventory/NotoItemDefinition.h"
#include "Misc/AutomationTest.h"
#include "Player/NotoPlayerState.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace NotoCombatTests
{
	template <typename PropertyType>
	PropertyType* FindPropertyChecked(const UClass* Class, const TCHAR* PropertyName)
	{
		PropertyType* Property = FindFProperty<PropertyType>(Class, PropertyName);
		check(Property);
		return Property;
	}

	template <typename EnumType>
	void SetEnumProperty(UObject& Object, const TCHAR* PropertyName, EnumType Value)
	{
		FEnumProperty* Property = FindPropertyChecked<FEnumProperty>(Object.GetClass(), PropertyName);
		Property->GetUnderlyingProperty()->SetIntPropertyValue(
			Property->ContainerPtrToValuePtr<void>(&Object), static_cast<int64>(Value));
	}

	void SetIntProperty(UObject& Object, const TCHAR* PropertyName, int32 Value)
	{
		FindPropertyChecked<FIntProperty>(Object.GetClass(), PropertyName)->SetPropertyValue_InContainer(&Object, Value);
	}

	void SetFloatProperty(UObject& Object, const TCHAR* PropertyName, float Value)
	{
		FindPropertyChecked<FFloatProperty>(Object.GetClass(), PropertyName)->SetPropertyValue_InContainer(&Object, Value);
	}

	void SetBoolProperty(UObject& Object, const TCHAR* PropertyName, bool bValue)
	{
		FindPropertyChecked<FBoolProperty>(Object.GetClass(), PropertyName)->SetPropertyValue_InContainer(
			&Object, bValue);
	}

	UNotoItemDefinition* MakeInternalFirearm(const TCHAR* Name, ENotoItemType ItemType, float FireInterval)
	{
		UNotoItemDefinition* Definition = NewObject<UNotoItemDefinition>(GetTransientPackage(), FName(Name));
		SetEnumProperty(*Definition, TEXT("ItemType"), ItemType);
		SetEnumProperty(*Definition, TEXT("AmmoFeedType"), ENotoAmmoFeedType::Internal);
		SetIntProperty(*Definition, TEXT("AmmoCapacity"), 10);
		SetBoolProperty(*Definition, TEXT("bFirearm"), true);
		SetFloatProperty(*Definition, TEXT("FireInterval"), FireInterval);
		SetFloatProperty(*Definition, TEXT("GunfireNoiseRange"), 0.0f);
		return Definition;
	}

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
	const FProperty* HealthProperty = FindFProperty<FProperty>(UNotoHealthSet::StaticClass(), TEXT("Health"));
	const FProperty* MaxHealthProperty = FindFProperty<FProperty>(UNotoHealthSet::StaticClass(), TEXT("MaxHealth"));
	TestTrue(TEXT("Health is a replicated attribute"), HealthProperty && HealthProperty->HasAnyPropertyFlags(CPF_Net));
	TestTrue(TEXT("MaxHealth is a replicated attribute"),
	         MaxHealthProperty && MaxHealthProperty->HasAnyPropertyFlags(CPF_Net));
	TestEqual(TEXT("Health uses its replication notification"),
	          HealthProperty ? HealthProperty->RepNotifyFunc : NAME_None,
	          FName(TEXT("OnRep_Health")));
	TestEqual(TEXT("MaxHealth uses its replication notification"),
	          MaxHealthProperty ? MaxHealthProperty->RepNotifyFunc : NAME_None,
	          FName(TEXT("OnRep_MaxHealth")));

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoFirearmPerItemCooldownTest,
	"NoTomorrow.Combat.Firearm.PerItemCooldown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoFirearmPerItemCooldownTest::RunTest(const FString& Parameters)
{
	using namespace NotoCombatTests;

	FTestWorld TestWorld;
	ANotoPlayerState* PlayerState = TestWorld.World->SpawnActor<ANotoPlayerState>();
	ANotoCharacter* Character = TestWorld.World->SpawnActor<ANotoCharacter>();
	TestNotNull(TEXT("PlayerState spawned"), PlayerState);
	TestNotNull(TEXT("Character spawned"), Character);
	if (!PlayerState || !Character)
	{
		return false;
	}

	Character->SetPlayerState(PlayerState);
	PlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(PlayerState, Character);
	UNotoInventoryComponent* Inventory = PlayerState->GetInventoryComponent();
	UNotoFirearmComponent* Firearm = Character->GetFirearmComponent();
	UNotoItemDefinition* MainWeapon = MakeInternalFirearm(TEXT("CooldownMainWeapon"), ENotoItemType::MainWeapon, 1.0f);
	UNotoItemDefinition* SecondaryWeapon = MakeInternalFirearm(
		TEXT("CooldownSecondaryWeapon"), ENotoItemType::SecondaryWeapon, 0.1f);

	FGuid MainWeaponId;
	FGuid SecondaryWeaponId;
	TestTrue(TEXT("Main weapon is added"), Inventory->AddItem(MainWeapon, 1, -1, MainWeaponId));
	TestTrue(TEXT("Secondary weapon is added"), Inventory->AddItem(SecondaryWeapon, 1, -1, SecondaryWeaponId));
	TestTrue(TEXT("Main weapon equips"),
	         Inventory->EquipItem(MainWeaponId, ENotoEquipmentSlot::MainWeapon, false));
	TestTrue(TEXT("Secondary weapon equips"),
	         Inventory->EquipItem(SecondaryWeaponId, ENotoEquipmentSlot::SecondaryWeapon, false));
	TestTrue(TEXT("Main weapon becomes active"), Inventory->SetActiveSlot(ENotoEquipmentSlot::MainWeapon));
	TestTrue(TEXT("Main weapon fires"), Firearm->TryFire());

	TestTrue(TEXT("Secondary weapon becomes active"), Inventory->SetActiveSlot(ENotoEquipmentSlot::SecondaryWeapon));
	TestTrue(TEXT("A different weapon is not blocked by the main weapon cooldown"), Firearm->TryFire());
	TestTrue(TEXT("Main weapon becomes active again"), Inventory->SetActiveSlot(ENotoEquipmentSlot::MainWeapon));
	TestFalse(TEXT("The original weapon retains its own cooldown"), Firearm->TryFire());
	return true;
}

#endif
