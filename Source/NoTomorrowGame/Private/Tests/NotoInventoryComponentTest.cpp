// © 2025 Kamenyari. All rights reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Inventory/NotoInventoryComponent.h"

#include "Development/NotoGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Inventory/NotoItemDefinition.h"
#include "Inventory/NotoItemPickup.h"
#include "Misc/AutomationTest.h"
#include "Misc/DataValidation.h"
#include "Player/NotoPlayerState.h"
#include "Tests/NotoInventoryTestListener.h"
#include "UObject/UnrealType.h"

namespace NotoInventoryTests
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
			Property->ContainerPtrToValuePtr<void>(&Object),
			static_cast<int64>(Value));
	}

	void SetIntProperty(UObject& Object, const TCHAR* PropertyName, int32 Value)
	{
		FindPropertyChecked<FIntProperty>(Object.GetClass(), PropertyName)->SetPropertyValue_InContainer(&Object, Value);
	}

	void SetBoolProperty(UObject& Object, const TCHAR* PropertyName, bool bValue)
	{
		FindPropertyChecked<FBoolProperty>(Object.GetClass(), PropertyName)->SetPropertyValue_InContainer(
			&Object, bValue);
	}

	void SetTagProperty(UObject& Object, const TCHAR* PropertyName, FGameplayTag Value)
	{
		FStructProperty* Property = FindPropertyChecked<FStructProperty>(Object.GetClass(), PropertyName);
		*Property->ContainerPtrToValuePtr<FGameplayTag>(&Object) = Value;
	}

	void SetObjectProperty(UObject& Object, const TCHAR* PropertyName, UObject* Value)
	{
		FindPropertyChecked<FObjectPropertyBase>(Object.GetClass(), PropertyName)->SetObjectPropertyValue_InContainer(
			&Object, Value);
	}

	UNotoItemDefinition* MakeDefinition(
		const TCHAR* Name,
		ENotoItemType ItemType,
		int32 MaxStackSize = 1,
		bool bDroppable = true)
	{
		UNotoItemDefinition* Definition = NewObject<UNotoItemDefinition>(GetTransientPackage(), FName(Name));
		SetEnumProperty(*Definition, TEXT("ItemType"), ItemType);
		SetIntProperty(*Definition, TEXT("MaxStackSize"), MaxStackSize);
		SetBoolProperty(*Definition, TEXT("bDroppable"), bDroppable);
		return Definition;
	}

	UNotoItemDefinition* MakeMagazine(const TCHAR* Name, FGameplayTag Family, int32 Capacity)
	{
		UNotoItemDefinition* Definition = MakeDefinition(Name, ENotoItemType::Magazine);
		SetTagProperty(*Definition, TEXT("AmmoFamily"), Family);
		SetIntProperty(*Definition, TEXT("MagazineCapacity"), Capacity);
		return Definition;
	}

	UNotoItemDefinition* MakeLooseAmmo(const TCHAR* Name, FGameplayTag Family, int32 MaxStackSize = 30)
	{
		UNotoItemDefinition* Definition = MakeDefinition(Name, ENotoItemType::Ammunition, MaxStackSize);
		SetTagProperty(*Definition, TEXT("AmmoFamily"), Family);
		return Definition;
	}

	UNotoItemDefinition* MakeMagazineWeapon(
		const TCHAR* Name,
		ENotoItemType ItemType,
		FGameplayTag Family,
		UNotoItemDefinition& StandardMagazine)
	{
		UNotoItemDefinition* Definition = MakeDefinition(Name, ItemType);
		SetEnumProperty(*Definition, TEXT("AmmoFeedType"), ENotoAmmoFeedType::DetachableMagazine);
		SetTagProperty(*Definition, TEXT("AmmoFamily"), Family);
		SetObjectProperty(*Definition, TEXT("StandardMagazineDefinition"), &StandardMagazine);
		SetBoolProperty(*Definition, TEXT("bFirearm"), true);
		return Definition;
	}

	UNotoItemDefinition* MakeInternalWeapon(
		const TCHAR* Name,
		ENotoItemType ItemType,
		FGameplayTag Family,
		int32 Capacity)
	{
		UNotoItemDefinition* Definition = MakeDefinition(Name, ItemType);
		SetEnumProperty(*Definition, TEXT("AmmoFeedType"), ENotoAmmoFeedType::Internal);
		SetTagProperty(*Definition, TEXT("AmmoFamily"), Family);
		SetIntProperty(*Definition, TEXT("InternalCapacity"), Capacity);
		SetBoolProperty(*Definition, TEXT("bFirearm"), true);
		return Definition;
	}

	void ClearFirstItemDefinition(UNotoInventoryComponent& Inventory)
	{
		FArrayProperty* ItemsProperty = FindPropertyChecked<FArrayProperty>(Inventory.GetClass(), TEXT("Items"));
		FScriptArrayHelper Items(ItemsProperty, ItemsProperty->ContainerPtrToValuePtr<void>(&Inventory));
		check(Items.Num() > 0);
		reinterpret_cast<FNotoItemInstance*>(Items.GetRawPtr(0))->Definition = nullptr;
	}

	FNotoItemInstance& GetMutableItem(UNotoInventoryComponent& Inventory, FGuid ItemInstanceId)
	{
		FArrayProperty* ItemsProperty = FindPropertyChecked<FArrayProperty>(Inventory.GetClass(), TEXT("Items"));
		FScriptArrayHelper Items(ItemsProperty, ItemsProperty->ContainerPtrToValuePtr<void>(&Inventory));
		for (int32 Index = 0; Index < Items.Num(); ++Index)
		{
			FNotoItemInstance& Item = *reinterpret_cast<FNotoItemInstance*>(Items.GetRawPtr(Index));
			if (Item.InstanceId == ItemInstanceId)
			{
				return Item;
			}
		}
		checkNoEntry();
		return *reinterpret_cast<FNotoItemInstance*>(Items.GetRawPtr(0));
	}

	struct FTestWorld
	{
		explicit FTestWorld(bool bCreatePawn = true)
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

			PlayerState = World->SpawnActor<ANotoPlayerState>();
			check(PlayerState);
			Inventory = PlayerState->GetInventoryComponent();
			check(Inventory);

			if (bCreatePawn)
			{
				Pawn = World->SpawnActor<APawn>();
				check(Pawn);
				Pawn->SetPlayerState(PlayerState);
			}
		}

		~FTestWorld()
		{
			World->CleanupWorld();
			GEngine->DestroyWorldContext(World);
			World->ReleasePhysicsScene();
		}

		int32 CountPickups() const
		{
			int32 Count = 0;
			for (TActorIterator<ANotoItemPickup> It(World); It; ++It)
			{
				++Count;
			}
			return Count;
		}

		UWorld* World = nullptr;
		ANotoPlayerState* PlayerState = nullptr;
		APawn* Pawn = nullptr;
		UNotoInventoryComponent* Inventory = nullptr;
	};

	FGuid Add(UNotoInventoryComponent& Inventory, UNotoItemDefinition& Definition, int32 Quantity,
	          int32 LoadedAmmo = -1)
	{
		FGuid ItemInstanceId;
		check(Inventory.AddItem(&Definition, Quantity, LoadedAmmo, ItemInstanceId));
		return ItemInstanceId;
	}

	bool TryCollect(
		UNotoInventoryComponent& Inventory,
		UNotoItemDefinition* Definition,
		int32 Quantity,
		int32 LoadedAmmo,
		FGuid& OutItemInstanceId,
		bool bMakeCollectedItemActive = true)
	{
		int32 RemainingLoadedAmmo = 0;
		return Inventory.CollectItem(
			Definition,
			Quantity,
			LoadedAmmo,
			OutItemInstanceId,
			RemainingLoadedAmmo,
			bMakeCollectedItemActive);
	}

	FGuid Collect(UNotoInventoryComponent& Inventory, UNotoItemDefinition& Definition, int32 Quantity = 1,
	              int32 LoadedAmmo = -1)
	{
		FGuid ItemInstanceId;
		check(TryCollect(Inventory, &Definition, Quantity, LoadedAmmo, ItemInstanceId));
		return ItemInstanceId;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryStackAndMagazineTest,
	"NoTomorrow.Inventory.StacksAndMagazines",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryStackAndMagazineTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	UNotoInventoryComponent* Inventory = NewObject<UNotoInventoryComponent>();
	UNotoItemDefinition* Stack = MakeDefinition(TEXT("Stack"), ENotoItemType::QuestItem, 3);
	const FGuid FirstStackId = Add(*Inventory, *Stack, 2);
	FGuid OverflowId;
	TestTrue(TEXT("Existing stack can be filled with overflow"), Inventory->AddItem(Stack, 4, 99, OverflowId));
	TestEqual(TEXT("The filled existing stack is returned"), OverflowId, FirstStackId);
	TestEqual(TEXT("Overflow creates exactly two stacks"), Inventory->GetItemsView().Num(), 2);
	TestEqual(TEXT("Non-ammunition stacks discard loaded ammo"), Inventory->GetItemsView()[0].LoadedAmmo, 0);

	UNotoItemDefinition* HeavyMagazine = MakeMagazine(
		TEXT("StackTestHeavyMagazine"), NotoGameplayTags::Ammo_Magazine_Heavy, 6);
	const FGuid FirstMagazineId = Add(*Inventory, *HeavyMagazine, 1, 4);
	const FGuid SecondMagazineId = Add(*Inventory, *HeavyMagazine, 1, 2);
	TestNotEqual(TEXT("Physical magazines preserve unique identities"), FirstMagazineId, SecondMagazineId);
	TestEqual(TEXT("Physical magazines never stack"), HeavyMagazine->GetMaxStackSize(), 1);
	TestTrue(TEXT("Magazine rounds can be changed"), Inventory->SetLoadedAmmo(FirstMagazineId, 3));
	TestFalse(TEXT("Magazine rounds cannot exceed definition capacity"), Inventory->SetLoadedAmmo(FirstMagazineId, 7));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryDefinitionResolutionTest,
	"NoTomorrow.Inventory.DefinitionResolution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryDefinitionResolutionTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	UNotoItemDefinition* Definition = LoadObject<UNotoItemDefinition>(
		nullptr,
		TEXT("/Game/Items/Definitions/DA_Tool_Test.DA_Tool_Test"));
	if (!TestNotNull(TEXT("Authored definition loads"), Definition))
	{
		return false;
	}

	UNotoInventoryComponent* Inventory = NewObject<UNotoInventoryComponent>();
	const FGuid ItemInstanceId = Add(*Inventory, *Definition, 1);
	ClearFirstItemDefinition(*Inventory);
	FNotoItemInstance Item;
	TestTrue(TEXT("Saved item remains addressable while unresolved"), Inventory->GetItem(ItemInstanceId, Item));
	TestNull(TEXT("Runtime definition starts unresolved"), Item.Definition.Get());
	TestTrue(TEXT("Definition ids resolve after load"), Inventory->ResolveItemDefinitions());
	TestTrue(TEXT("Resolved item remains addressable"), Inventory->GetItem(ItemInstanceId, Item));
	TestEqual(TEXT("Resolved definition matches authored asset"), Item.Definition.Get(), Definition);
	return true;
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoItemDefinitionValidationTest,
	"NoTomorrow.Inventory.DefinitionValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoItemDefinitionValidationTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	UNotoItemDefinition* HeavyMagazine = MakeMagazine(
		TEXT("ValidHeavyMagazine"), NotoGameplayTags::Ammo_Magazine_Heavy, 30);
	UNotoItemDefinition* ValidWeapon = MakeMagazineWeapon(
		TEXT("ValidHeavyWeapon"), ENotoItemType::MainWeapon, NotoGameplayTags::Ammo_Magazine_Heavy,
		*HeavyMagazine);
	FDataValidationContext ValidContext;
	TestEqual(TEXT("Matching physical magazine weapon is valid"), ValidWeapon->IsDataValid(ValidContext),
	          EDataValidationResult::Valid);

	UNotoItemDefinition* WrongFamilyWeapon = MakeMagazineWeapon(
		TEXT("WrongFamilyWeapon"), ENotoItemType::MainWeapon, NotoGameplayTags::Ammo_Magazine_Light,
		*HeavyMagazine);
	FDataValidationContext WrongFamilyContext;
	TestEqual(TEXT("Mismatched standard magazine family is invalid"),
	          WrongFamilyWeapon->IsDataValid(WrongFamilyContext), EDataValidationResult::Invalid);

	UNotoItemDefinition* StackedMagazine = MakeMagazine(
		TEXT("StackedMagazine"), NotoGameplayTags::Ammo_Magazine_Heavy, 30);
	SetIntProperty(*StackedMagazine, TEXT("MaxStackSize"), 2);
	FDataValidationContext StackedContext;
	TestEqual(TEXT("Physical magazines cannot be authored stackable"),
	          StackedMagazine->IsDataValid(StackedContext), EDataValidationResult::Invalid);

	UNotoItemDefinition* StackedMain = MakeDefinition(TEXT("StackedMain"), ENotoItemType::MainWeapon, 2);
	FDataValidationContext StackedMainContext;
	TestEqual(TEXT("Authored stackable main weapon is invalid"), StackedMain->IsDataValid(StackedMainContext),
	          EDataValidationResult::Invalid);
	return true;
}
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryCollectionPlanningTest,
	"NoTomorrow.Inventory.CollectionPlanning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryCollectionPlanningTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld TestWorld;
	UNotoItemDefinition* Tool = MakeDefinition(TEXT("PreferredStackTool"), ENotoItemType::Tool, 5);
	const FGuid EarlierUnequippedStack = Add(*TestWorld.Inventory, *Tool, 5);
	const FGuid EquippedStack = Add(*TestWorld.Inventory, *Tool, 2);
	TestTrue(TEXT("Second stack equips"), TestWorld.Inventory->EquipItem(
		         EquippedStack, ENotoEquipmentSlot::Tool5, false));
	TestTrue(TEXT("Partial drop creates room in earlier stack"), TestWorld.Inventory->DropItemAt(
		         EarlierUnequippedStack, 2, FTransform(FVector(500.0f, 0.0f, 0.0f))));

	FGuid CollectedId;
	TestTrue(TEXT("Matching equipped stack collection succeeds"), TryCollect(
		         *TestWorld.Inventory, Tool, 5, -1, CollectedId));
	TestEqual(TEXT("Matching equipped stack is returned"), CollectedId, EquippedStack);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		Collect(*TestWorld.Inventory, *MakeDefinition(
			        *FString::Printf(TEXT("FilledTool%d"), Index), ENotoItemType::Tool));
	}
	UNotoItemDefinition* OverflowTool = MakeDefinition(TEXT("OverflowTool"), ENotoItemType::Tool);
	const FGuid OverflowToolId = Collect(*TestWorld.Inventory, *OverflowTool);
	FNotoItemInstance EquippedItem;
	TestFalse(TEXT("Overflow tool is not equipped"), TestWorld.Inventory->GetEquippedItem(
		          ENotoEquipmentSlot::Tool1, EquippedItem) && EquippedItem.InstanceId == OverflowToolId);
	TestEqual(TEXT("Occupied slots cause no automatic drop"), TestWorld.CountPickups(), 1);
	TestEqual(TEXT("Overflow equipment remains regular inventory"), TestWorld.Inventory->GetTotalQuantity(OverflowTool),
	          1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryActiveSlotPreservationTest,
	"NoTomorrow.Inventory.ActiveSlotPreservation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryActiveSlotPreservationTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld TestWorld;
	UNotoItemDefinition* MainWeapon = MakeDefinition(TEXT("ActiveMain"), ENotoItemType::MainWeapon);
	UNotoItemDefinition* SecondaryWeapon = MakeDefinition(TEXT("ActiveSecondary"), ENotoItemType::SecondaryWeapon);
	UNotoItemDefinition* ExtraSecondary = MakeDefinition(TEXT("ExtraSecondary"), ENotoItemType::SecondaryWeapon);
	Collect(*TestWorld.Inventory, *MainWeapon);
	const FGuid EquippedSecondaryId = Collect(*TestWorld.Inventory, *SecondaryWeapon);
	TestTrue(TEXT("Main weapon can become active"), TestWorld.Inventory->SetActiveSlot(ENotoEquipmentSlot::MainWeapon));

	FGuid ExtraSecondaryId;
	TestTrue(TEXT("Occupied secondary slot accepts pickup into inventory"), TryCollect(
		         *TestWorld.Inventory, ExtraSecondary, 1, -1, ExtraSecondaryId, false));
	FNotoItemInstance EquippedSecondary;
	TestTrue(TEXT("Original secondary remains equipped"), TestWorld.Inventory->GetEquippedItem(
		         ENotoEquipmentSlot::SecondaryWeapon, EquippedSecondary));
	TestEqual(TEXT("Original secondary identity is preserved"), EquippedSecondary.InstanceId, EquippedSecondaryId);
	TestNotEqual(TEXT("Extra secondary is a distinct inventory item"), ExtraSecondaryId, EquippedSecondaryId);
	TestEqual(TEXT("Inventory-only pickup preserves active main slot"), TestWorld.Inventory->GetActiveSlot(),
	          ENotoEquipmentSlot::MainWeapon);
	TestEqual(TEXT("Inventory-only pickup drops nothing"), TestWorld.CountPickups(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryDropAndNotificationTest,
	"NoTomorrow.Inventory.DropAndNotifications",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryDropAndNotificationTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld TestWorld;
	UNotoInventoryTestListener* Listener = NewObject<UNotoInventoryTestListener>();
	Listener->Initialize(TestWorld.Inventory);
	TestWorld.Inventory->OnInventoryChanged.AddDynamic(Listener, &UNotoInventoryTestListener::HandleInventoryChanged);
	TestWorld.Inventory->OnEquipmentChanged.AddDynamic(Listener, &UNotoInventoryTestListener::HandleEquipmentChanged);

	UNotoItemDefinition* Stack = MakeDefinition(TEXT("DropStack"), ENotoItemType::Tool, 5);
	const FGuid StackId = Add(*TestWorld.Inventory, *Stack, 5);
	TestEqual(TEXT("AddItem emits one inventory callback"), Listener->InventoryChangedCount, 1);
	Listener->Reset();
	TestTrue(TEXT("Stack equips"), TestWorld.Inventory->EquipItem(StackId, ENotoEquipmentSlot::Tool5, false));
	TestEqual(TEXT("Equip emits one equipment callback"), Listener->EquipmentChangedCount, 1);
	Listener->Reset();
	TestTrue(TEXT("Equipped item drops"), TestWorld.Inventory->DropItem(StackId, 5));
	TestEqual(TEXT("Full drop emits one inventory callback"), Listener->InventoryChangedCount, 1);
	TestEqual(TEXT("Full drop emits one equipment callback"), Listener->EquipmentChangedCount, 1);
	TestEqual(TEXT("Full drop repairs active slot"), TestWorld.Inventory->GetActiveSlot(), ENotoEquipmentSlot::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryPickupAgreementTest,
	"NoTomorrow.Inventory.PickupAgreement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryPickupAgreementTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld TestWorld;
	UNotoItemDefinition* HeavyMagazine = MakeMagazine(
		TEXT("PickupHeavyMagazine"), NotoGameplayTags::Ammo_Magazine_Heavy, 6);
	UNotoItemDefinition* Weapon = MakeMagazineWeapon(
		TEXT("PickupWeapon"), ENotoItemType::MainWeapon, NotoGameplayTags::Ammo_Magazine_Heavy, *HeavyMagazine);
	const FGuid EquippedWeaponId = Collect(*TestWorld.Inventory, *Weapon, 1, 2);

	ANotoItemPickup* DuplicatePickup = TestWorld.World->SpawnActor<ANotoItemPickup>();
	DuplicatePickup->InitializePickup(Weapon, 1, 4);
	TestTrue(TEXT("Duplicate weapon can be collected as regular inventory"),
	         DuplicatePickup->CanInteract_Implementation(TestWorld.Pawn));
	DuplicatePickup->Interact_Implementation(TestWorld.Pawn, FNotoInteractionRequest());
	TestTrue(TEXT("Collected duplicate pickup is destroyed"), DuplicatePickup->IsActorBeingDestroyed());
	TestEqual(TEXT("Duplicate weapon remains a second physical weapon"), TestWorld.Inventory->GetTotalQuantity(Weapon),
	          2);
	FNotoItemInstance EquippedWeapon;
	TestTrue(TEXT("Original weapon remains equipped"), TestWorld.Inventory->GetEquippedItem(
		         ENotoEquipmentSlot::MainWeapon, EquippedWeapon));
	TestEqual(TEXT("Original equipped weapon identity is unchanged"), EquippedWeapon.InstanceId, EquippedWeaponId);
	TestEqual(TEXT("Original inserted magazine was not refilled"), EquippedWeapon.InsertedMagazine.LoadedAmmo, 2);

	ANotoItemPickup* InvalidPickup = TestWorld.World->SpawnActor<ANotoItemPickup>();
	InvalidPickup->InitializePickup(Weapon, 2, 4);
	TestFalse(TEXT("Unique weapon pickup rejects quantity two"), InvalidPickup->CanInteract_Implementation(TestWorld.Pawn));
	InvalidPickup->Interact_Implementation(TestWorld.Pawn, FNotoInteractionRequest());
	TestFalse(TEXT("Rejected pickup remains in the world"), InvalidPickup->IsActorBeingDestroyed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryPhysicalAmmunitionTest,
	"NoTomorrow.Inventory.PhysicalAmmunition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryPhysicalAmmunitionTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld TestWorld;
	UNotoItemDefinition* HeavyMagazine = MakeMagazine(
		TEXT("PhysicalHeavyMagazine"), NotoGameplayTags::Ammo_Magazine_Heavy, 6);
	UNotoItemDefinition* HeavyWeapon = MakeMagazineWeapon(
		TEXT("PhysicalHeavyWeapon"), ENotoItemType::MainWeapon, NotoGameplayTags::Ammo_Magazine_Heavy,
		*HeavyMagazine);
	const FGuid WeaponId = Collect(*TestWorld.Inventory, *HeavyWeapon, 1, 2);
	const FGuid FourRoundMagazineId = Add(*TestWorld.Inventory, *HeavyMagazine, 1, 4);
	const FGuid FullMagazineId = Add(*TestWorld.Inventory, *HeavyMagazine, 1, 6);
	const FGuid FiveRoundMagazineId = Add(*TestWorld.Inventory, *HeavyMagazine, 1, 5);

	FNotoItemInstance Weapon;
	TestTrue(TEXT("Weapon is present"), TestWorld.Inventory->GetItem(WeaponId, Weapon));
	FNotoItemInstance InvalidDuplicateMagazine = Weapon;
	InvalidDuplicateMagazine.InstanceId = FGuid::NewGuid();
	TestFalse(TEXT("A collected weapon cannot duplicate an inserted magazine GUID"),
	          TestWorld.Inventory->CanCollectItemInstance(InvalidDuplicateMagazine));
	const FGuid OriginalMagazineId = Weapon.InsertedMagazine.InstanceId;
	int32 ReloadedRounds = 0;
	TestTrue(TEXT("Reload selects a compatible spare magazine"), TestWorld.Inventory->ReloadItem(
		         WeaponId, ReloadedRounds));
	TestEqual(TEXT("Reload selects the fullest spare"), ReloadedRounds, 6);
	TestTrue(TEXT("Reloaded weapon is present"), TestWorld.Inventory->GetItem(WeaponId, Weapon));
	TestEqual(TEXT("Inserted magazine keeps the selected spare GUID"), Weapon.InsertedMagazine.InstanceId,
	          FullMagazineId);
	FNotoItemInstance ReturnedMagazine;
	TestTrue(TEXT("Tactical reload returns the partial old magazine"), TestWorld.Inventory->GetItem(
		         OriginalMagazineId, ReturnedMagazine));
	TestEqual(TEXT("Returned magazine preserves its rounds"), ReturnedMagazine.LoadedAmmo, 2);
	TestFalse(TEXT("Full magazine cannot be replaced by a less-full spare"), TestWorld.Inventory->ReloadItem(
		          WeaponId, ReloadedRounds));

	TestTrue(TEXT("A full magazine can be consumed"), TestWorld.Inventory->ConsumeLoadedAmmo(WeaponId, 6));
	TestTrue(TEXT("Reload replaces an empty inserted magazine"), TestWorld.Inventory->ReloadItem(
		         WeaponId, ReloadedRounds));
	TestTrue(TEXT("Reloaded weapon remains present"), TestWorld.Inventory->GetItem(WeaponId, Weapon));
	TestEqual(TEXT("Next-fullest spare is selected"), Weapon.InsertedMagazine.InstanceId, FiveRoundMagazineId);
	TestFalse(TEXT("Removed empty magazine is discarded"), TestWorld.Inventory->GetItem(
		          FullMagazineId, ReturnedMagazine));
	TestTrue(TEXT("Lower partial magazine is preserved"), TestWorld.Inventory->GetItem(
		         FourRoundMagazineId, ReturnedMagazine));

	UNotoItemDefinition* Shells = MakeLooseAmmo(TEXT("Shells"), NotoGameplayTags::Ammo_Shell, 20);
	UNotoItemDefinition* Shotgun = MakeInternalWeapon(
		TEXT("Shotgun"), ENotoItemType::SecondaryWeapon, NotoGameplayTags::Ammo_Shell, 2);
	const FGuid ShotgunId = Add(*TestWorld.Inventory, *Shotgun, 1, 0);
	const FGuid ShellStackId = Add(*TestWorld.Inventory, *Shells, 3);
	TestTrue(TEXT("Internal reload consumes one shell"), TestWorld.Inventory->ReloadItem(ShotgunId, ReloadedRounds));
	TestEqual(TEXT("Internal reload reports one shell"), ReloadedRounds, 1);
	TestTrue(TEXT("Second shell can be loaded"), TestWorld.Inventory->ReloadItem(ShotgunId, ReloadedRounds));
	TestFalse(TEXT("Full internal feed rejects another reload"), TestWorld.Inventory->ReloadItem(
		          ShotgunId, ReloadedRounds));
	FNotoItemInstance ShellStack;
	TestTrue(TEXT("Remaining shell stack exists"), TestWorld.Inventory->GetItem(ShellStackId, ShellStack));
	TestEqual(TEXT("Two individual shells were consumed"), ShellStack.Quantity, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryWeaponDropIdentityTest,
	"NoTomorrow.Inventory.WeaponDropIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryWeaponDropIdentityTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld TestWorld;
	UNotoItemDefinition* LightMagazine = MakeMagazine(
		TEXT("DropLightMagazine"), NotoGameplayTags::Ammo_Magazine_Light, 5);
	UNotoItemDefinition* WeaponDefinition = MakeMagazineWeapon(
		TEXT("DropLightWeapon"), ENotoItemType::MainWeapon, NotoGameplayTags::Ammo_Magazine_Light,
		*LightMagazine);
	const FGuid WeaponId = Collect(*TestWorld.Inventory, *WeaponDefinition, 1, 3);
	FNotoItemInstance Weapon;
	TestTrue(TEXT("Weapon exists before drop"), TestWorld.Inventory->GetItem(WeaponId, Weapon));
	const FGuid MagazineId = Weapon.InsertedMagazine.InstanceId;
	TestTrue(TEXT("Weapon drops with inserted magazine"), TestWorld.Inventory->DropItem(WeaponId, 1));

	const TActorIterator<ANotoItemPickup> PickupIt(TestWorld.World);
	ANotoItemPickup* Pickup = PickupIt ? *PickupIt : nullptr;
	TestNotNull(TEXT("Dropped weapon creates a pickup"), Pickup);
	if (Pickup)
	{
		Pickup->Interact_Implementation(TestWorld.Pawn, FNotoInteractionRequest());
		TestTrue(TEXT("Picked-up weapon restores its original identity"), TestWorld.Inventory->GetItem(WeaponId, Weapon));
		TestEqual(TEXT("Inserted magazine GUID survives drop and pickup"), Weapon.InsertedMagazine.InstanceId,
		          MagazineId);
		TestEqual(TEXT("Inserted magazine rounds survive drop and pickup"), Weapon.InsertedMagazine.LoadedAmmo, 3);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryLegacyAmmunitionMigrationTest,
	"NoTomorrow.Inventory.LegacyAmmunitionMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryLegacyAmmunitionMigrationTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	UNotoInventoryComponent* Inventory = NewObject<UNotoInventoryComponent>();
	UNotoItemDefinition* HeavyMagazine = MakeMagazine(
		TEXT("LegacyHeavyMagazine"), NotoGameplayTags::Ammo_Magazine_Heavy, 6);
	UNotoItemDefinition* HeavyWeapon = MakeMagazineWeapon(
		TEXT("LegacyHeavyWeapon"), ENotoItemType::MainWeapon, NotoGameplayTags::Ammo_Magazine_Heavy,
		*HeavyMagazine);
	const FGuid WeaponId = Add(*Inventory, *HeavyWeapon, 1, 0);
	FNotoItemInstance& LegacyWeapon = GetMutableItem(*Inventory, WeaponId);
	LegacyWeapon.InsertedMagazine.Reset();
	LegacyWeapon.LoadedAmmo = 8;
	LegacyWeapon.ReserveAmmo = 8;

	TestTrue(TEXT("Legacy state resolves and migrates"), Inventory->ResolveItemDefinitions());
	FNotoItemInstance MigratedWeapon;
	TestTrue(TEXT("Migrated weapon remains addressable"), Inventory->GetItem(WeaponId, MigratedWeapon));
	TestTrue(TEXT("Legacy loaded ammo gains a physical magazine identity"),
	         MigratedWeapon.InsertedMagazine.InstanceId.IsValid());
	TestEqual(TEXT("Legacy loaded rounds move into inserted magazine"),
	          MigratedWeapon.InsertedMagazine.LoadedAmmo, 6);
	TestEqual(TEXT("Legacy reserve becomes two physical magazines"), Inventory->GetTotalQuantity(HeavyMagazine), 2);
	int32 MigratedReserveRounds = 0;
	for (const FNotoItemInstance& Item : Inventory->GetItemsView())
	{
		if (Item.Definition == HeavyMagazine)
		{
			MigratedReserveRounds += Item.LoadedAmmo;
		}
	}
	TestEqual(TEXT("Legacy loaded overflow and reserve rounds are conserved"), MigratedReserveRounds, 10);
	TestEqual(TEXT("Deprecated reserve is cleared after migration"), MigratedWeapon.ReserveAmmo, 0);
	return true;
}

#endif
