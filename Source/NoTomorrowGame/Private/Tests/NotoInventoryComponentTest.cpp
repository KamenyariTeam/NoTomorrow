// © 2025 Kamenyari. All rights reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Inventory/NotoInventoryComponent.h"

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

	void SetItemType(UNotoItemDefinition& Definition, ENotoItemType ItemType)
	{
		FEnumProperty* Property = FindPropertyChecked<FEnumProperty>(Definition.GetClass(), TEXT("ItemType"));
		Property->GetUnderlyingProperty()->SetIntPropertyValue(
			Property->ContainerPtrToValuePtr<void>(&Definition),
			static_cast<int64>(ItemType));
	}

	void SetIntProperty(UObject& Object, const TCHAR* PropertyName, int32 Value)
	{
		FindPropertyChecked<FIntProperty>(Object.GetClass(), PropertyName)->
			SetPropertyValue_InContainer(&Object, Value);
	}

	void SetBoolProperty(UObject& Object, const TCHAR* PropertyName, bool bValue)
	{
		FindPropertyChecked<FBoolProperty>(Object.GetClass(), PropertyName)->SetPropertyValue_InContainer(
			&Object, bValue);
	}

	UNotoItemDefinition* MakeDefinition(
		const TCHAR* Name,
		ENotoItemType ItemType,
		int32 MaxStackSize = 1,
		bool bDroppable = true,
		int32 MagazineCapacity = 0)
	{
		UNotoItemDefinition* Definition = NewObject<UNotoItemDefinition>(GetTransientPackage(), FName(Name));
		SetItemType(*Definition, ItemType);
		SetIntProperty(*Definition, TEXT("MaxStackSize"), MaxStackSize);
		SetBoolProperty(*Definition, TEXT("bDroppable"), bDroppable);
		SetIntProperty(*Definition, TEXT("MagazineCapacity"), MagazineCapacity);
		return Definition;
	}

	void SetFirstItemQuantity(UNotoInventoryComponent& Inventory, int32 Quantity)
	{
		FArrayProperty* ItemsProperty = FindPropertyChecked<FArrayProperty>(Inventory.GetClass(), TEXT("Items"));
		FScriptArrayHelper Items(ItemsProperty, ItemsProperty->ContainerPtrToValuePtr<void>(&Inventory));
		check(Items.Num() > 0);
		reinterpret_cast<FNotoItemInstance*>(Items.GetRawPtr(0))->Quantity = Quantity;
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

	FGuid Collect(UNotoInventoryComponent& Inventory, UNotoItemDefinition& Definition, int32 Quantity = 1)
	{
		FGuid ItemInstanceId;
		check(Inventory.CollectItem(&Definition, Quantity, -1, ItemInstanceId));
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
	TestEqual(TEXT("First stack is full"), Inventory->GetItemsView()[0].Quantity, 3);
	TestEqual(TEXT("Overflow stack receives the remainder"), Inventory->GetItemsView()[1].Quantity, 3);
	TestEqual(TEXT("Non-magazine stacks discard loaded ammo"), Inventory->GetItemsView()[0].LoadedAmmo, 0);

	UNotoItemDefinition* StackableSecondary = MakeDefinition(
		TEXT("StackableSecondary"), ENotoItemType::SecondaryWeapon, 4);
	UNotoInventoryComponent* SecondaryInventory = NewObject<UNotoInventoryComponent>();
	Add(*SecondaryInventory, *StackableSecondary, 6, -1);
	TestEqual(TEXT("A non-magazine secondary uses its authored stack size"), StackableSecondary->GetMaxStackSize(), 4);
	TestEqual(TEXT("Stackable secondary creates two stacks"), SecondaryInventory->GetItemsView().Num(), 2);
	TestEqual(TEXT("Stackable secondary has no loaded ammo"), SecondaryInventory->GetItemsView()[0].LoadedAmmo, 0);

	UNotoItemDefinition* MagazineSecondary = MakeDefinition(
		TEXT("MagazineSecondary"), ENotoItemType::SecondaryWeapon, 5, true, 6);
	UNotoInventoryComponent* MagazineInventory = NewObject<UNotoInventoryComponent>();
	const FGuid MagazineId = Add(*MagazineInventory, *MagazineSecondary, 2, -1);
	TestTrue(TEXT("Secondary with magazine reports magazine state"), MagazineSecondary->UsesMagazine());
	TestEqual(TEXT("Magazine secondary is effectively unique"), MagazineSecondary->GetMaxStackSize(), 1);
	TestEqual(TEXT("Two magazine secondaries create unique instances"), MagazineInventory->GetItemsView().Num(), 2);
	TestEqual(TEXT("Negative loaded ammo initializes a full magazine"), MagazineInventory->GetItemsView()[0].LoadedAmmo,
	          6);
	TestTrue(TEXT("Magazine ammo can be changed"), MagazineInventory->SetLoadedAmmo(MagazineId, 3));
	TestFalse(
		TEXT("Loaded ammo is rejected on stackable secondaries"),
		SecondaryInventory->SetLoadedAmmo(SecondaryInventory->GetItemsView()[0].InstanceId, 1));

	UNotoItemDefinition* UniqueSecondary = MakeDefinition(TEXT("UniqueSecondary"), ENotoItemType::SecondaryWeapon, 1);
	UNotoInventoryComponent* UniqueInventory = NewObject<UNotoInventoryComponent>();
	const FGuid UniqueId = Add(*UniqueInventory, *UniqueSecondary, 1, -1);
	TestEqual(
		TEXT("Unique non-magazine secondary initializes loaded ammo to zero"),
		UniqueInventory->GetItemsView()[0].LoadedAmmo, 0);
	TestFalse(
		TEXT("Loaded ammo is rejected on a unique non-magazine secondary"),
		UniqueInventory->SetLoadedAmmo(UniqueId, 0));

	SetFirstItemQuantity(*MagazineInventory, 2);
	TestFalse(
		TEXT("Loaded ammo rejects an invalid stacked magazine instance"),
		MagazineInventory->SetLoadedAmmo(MagazineId, 2));

	FTestWorld DuplicateWeaponWorld;
	UNotoItemDefinition* DuplicateWeapon = MakeDefinition(
		TEXT("DuplicateWeapon"), ENotoItemType::MainWeapon, 1, true, 6);
	FGuid EquippedWeaponId;
	TestTrue(TEXT("Initial weapon collection succeeds"), DuplicateWeaponWorld.Inventory->CollectItem(
		         DuplicateWeapon, 1, 2, EquippedWeaponId));
	TestTrue(
		TEXT("Duplicate weapon can transfer its loaded rounds"),
		DuplicateWeaponWorld.Inventory->CanCollectItem(DuplicateWeapon, 1, 4));
	FGuid RefilledWeaponId;
	TestTrue(
		TEXT("Duplicate weapon collection transfers rounds"),
		DuplicateWeaponWorld.Inventory->CollectItem(DuplicateWeapon, 1, 4, RefilledWeaponId));
	TestEqual(TEXT("Duplicate collection returns the equipped weapon"), RefilledWeaponId, EquippedWeaponId);
	TestEqual(
		TEXT("Duplicate collection does not create another weapon"),
		DuplicateWeaponWorld.Inventory->GetItemsView().Num(), 1);
	TestEqual(
		TEXT("Duplicate collection fills the magazine"), DuplicateWeaponWorld.Inventory->GetItemsView()[0].LoadedAmmo,
		6);
	TestEqual(
		TEXT("Duplicate collection activates the matching weapon"), DuplicateWeaponWorld.Inventory->GetActiveSlot(),
		ENotoEquipmentSlot::MainWeapon);
	TestFalse(
		TEXT("A full matching weapon cannot consume another duplicate"),
		DuplicateWeaponWorld.Inventory->CanCollectItem(DuplicateWeapon, 1, 1));
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

	UNotoItemDefinition* StackedMagazine = MakeDefinition(
		TEXT("StackedMagazine"), ENotoItemType::SecondaryWeapon, 3, true, 6);
	FDataValidationContext StackedMagazineContext;
	TestEqual(
		TEXT("Stackable secondary with magazine data is invalid"), StackedMagazine->IsDataValid(StackedMagazineContext),
		EDataValidationResult::Invalid);

	UNotoItemDefinition* NonWeaponMagazine = MakeDefinition(TEXT("NonWeaponMagazine"), ENotoItemType::Tool, 1, true, 6);
	FDataValidationContext NonWeaponContext;
	TestEqual(TEXT("Magazine data on a non-weapon is invalid"), NonWeaponMagazine->IsDataValid(NonWeaponContext),
	          EDataValidationResult::Invalid);

	UNotoItemDefinition* StackedMain = MakeDefinition(TEXT("StackedMain"), ENotoItemType::MainWeapon, 2);
	FDataValidationContext StackedMainContext;
	TestEqual(TEXT("Authored stackable main weapon is invalid"), StackedMain->IsDataValid(StackedMainContext),
	          EDataValidationResult::Invalid);
	TestEqual(TEXT("Runtime still forces main weapons unique"), StackedMain->GetMaxStackSize(), 1);

	UNotoItemDefinition* NegativeMagazine = MakeDefinition(
		TEXT("NegativeMagazine"), ENotoItemType::SecondaryWeapon, 1, true, -1);
	FDataValidationContext NegativeMagazineContext;
	TestEqual(TEXT("Negative magazine capacity is invalid"), NegativeMagazine->IsDataValid(NegativeMagazineContext),
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
	TestTrue(TEXT("Partial drop creates room in the earlier unequipped stack"),
	         TestWorld.Inventory->DropItemAt(EarlierUnequippedStack, 2, FTransform(FVector(500.0f, 0.0f, 0.0f))));

	FGuid CollectedId;
	TestTrue(
		TEXT("Matching equipped stack collection succeeds"),
		TestWorld.Inventory->CollectItem(Tool, 5, -1, CollectedId));
	TestEqual(TEXT("Matching equipped stack is returned"), CollectedId, EquippedStack);
	FNotoItemInstance EarlierItem;
	FNotoItemInstance EquippedItem;
	TestTrue(TEXT("Earlier stack remains"), TestWorld.Inventory->GetItem(EarlierUnequippedStack, EarlierItem));
	TestTrue(TEXT("Equipped stack remains"), TestWorld.Inventory->GetItem(EquippedStack, EquippedItem));
	TestEqual(TEXT("Equipped stack fills before earlier matching stack"), EquippedItem.Quantity, 5);
	TestEqual(TEXT("Overflow then fills the earlier stack"), EarlierItem.Quantity, 5);
	TestEqual(TEXT("Matching slot becomes active"), TestWorld.Inventory->GetActiveSlot(), ENotoEquipmentSlot::Tool5);
	TestEqual(TEXT("No replacement pickup was spawned"), TestWorld.CountPickups(), 1);

	FTestWorld EmptySlotWorld;
	UNotoItemDefinition* FirstTool = MakeDefinition(TEXT("FirstEmptyTool"), ENotoItemType::Tool);
	UNotoItemDefinition* SecondTool = MakeDefinition(TEXT("SecondEmptyTool"), ENotoItemType::Tool);
	Collect(*EmptySlotWorld.Inventory, *FirstTool);
	Collect(*EmptySlotWorld.Inventory, *SecondTool);
	FNotoItemInstance SlotItem;
	TestTrue(TEXT("First tool chooses Tool5"), EmptySlotWorld.Inventory->GetEquippedItem(
		         ENotoEquipmentSlot::Tool5, SlotItem));
	TestTrue(TEXT("Tool5 contains first tool"), SlotItem.Definition == FirstTool);
	TestTrue(
		TEXT("Second tool chooses Tool4 before replacement"),
		EmptySlotWorld.Inventory->GetEquippedItem(ENotoEquipmentSlot::Tool4, SlotItem));
	TestTrue(TEXT("Tool4 contains second tool"), SlotItem.Definition == SecondTool);
	TestEqual(TEXT("Empty slots do not drop items"), EmptySlotWorld.CountPickups(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryToolReplacementTest,
	"NoTomorrow.Inventory.ToolReplacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryToolReplacementTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld ActiveReplacementWorld;
	TArray<UNotoItemDefinition*> InitialTools;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		UNotoItemDefinition* Tool = MakeDefinition(*FString::Printf(TEXT("ActiveReplacementTool%d"), Index),
		                                           ENotoItemType::Tool);
		InitialTools.Add(Tool);
		Collect(*ActiveReplacementWorld.Inventory, *Tool);
	}
	TestEqual(TEXT("Fifth collected tool is active in Tool1"), ActiveReplacementWorld.Inventory->GetActiveSlot(),
	          ENotoEquipmentSlot::Tool1);
	UNotoItemDefinition* Replacement = MakeDefinition(TEXT("ActiveReplacement"), ENotoItemType::Tool);
	Collect(*ActiveReplacementWorld.Inventory, *Replacement);
	FNotoItemInstance EquippedItem;
	TestTrue(TEXT("Replacement remains in active Tool1"), ActiveReplacementWorld.Inventory->GetEquippedItem(
		         ENotoEquipmentSlot::Tool1, EquippedItem));
	TestTrue(TEXT("Active droppable tool is replaced"), EquippedItem.Definition == Replacement);
	TestEqual(TEXT("One replaced tool becomes a pickup"), ActiveReplacementWorld.CountPickups(), 1);

	FTestWorld SkipNonDroppableWorld;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		const bool bDroppable = Index == 0;
		UNotoItemDefinition* Tool = MakeDefinition(
			*FString::Printf(TEXT("SkipNonDroppableTool%d"), Index),
			ENotoItemType::Tool,
			1,
			bDroppable);
		Collect(*SkipNonDroppableWorld.Inventory, *Tool);
	}
	FNotoItemInstance OriginalActiveItem;
	TestTrue(TEXT("Non-droppable active tool exists"), SkipNonDroppableWorld.Inventory->GetEquippedItem(
		         ENotoEquipmentSlot::Tool1, OriginalActiveItem));
	UNotoItemDefinition* FallbackReplacement = MakeDefinition(TEXT("FallbackReplacement"), ENotoItemType::Tool);
	Collect(*SkipNonDroppableWorld.Inventory, *FallbackReplacement);
	TestTrue(
		TEXT("Active non-droppable tool is retained"),
		SkipNonDroppableWorld.Inventory->GetEquippedItem(ENotoEquipmentSlot::Tool1, EquippedItem));
	TestEqual(TEXT("Retained active tool has the same id"), EquippedItem.InstanceId, OriginalActiveItem.InstanceId);
	TestTrue(
		TEXT("First droppable slot Tool5 is replaced"),
		SkipNonDroppableWorld.Inventory->GetEquippedItem(ENotoEquipmentSlot::Tool5, EquippedItem));
	TestTrue(TEXT("Fallback replacement is in Tool5"), EquippedItem.Definition == FallbackReplacement);

	FTestWorld FailureWorld;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		UNotoItemDefinition* Tool = MakeDefinition(
			*FString::Printf(TEXT("NonDroppableTool%d"), Index),
			ENotoItemType::Tool,
			1,
			false);
		Collect(*FailureWorld.Inventory, *Tool);
	}
	const TArray<FNotoItemInstance> ItemsBefore = FailureWorld.Inventory->GetItems();
	const TArray<FNotoEquippedItem> EquipmentBefore = FailureWorld.Inventory->GetEquipment();
	const ENotoEquipmentSlot ActiveSlotBefore = FailureWorld.Inventory->GetActiveSlot();
	UNotoItemDefinition* BlockedReplacement = MakeDefinition(TEXT("BlockedReplacement"), ENotoItemType::Tool);
	FGuid FailedItemId;
	TestFalse(TEXT("Collection fails when every replacement is non-droppable"),
	          FailureWorld.Inventory->CollectItem(BlockedReplacement, 1, -1, FailedItemId));
	TestEqual(TEXT("Failed collection does not change item count"), FailureWorld.Inventory->GetItemsView().Num(),
	          ItemsBefore.Num());
	TestEqual(
		TEXT("Failed collection does not change equipment count"), FailureWorld.Inventory->GetEquipmentView().Num(),
		EquipmentBefore.Num());
	TestEqual(TEXT("Failed collection preserves active slot"), FailureWorld.Inventory->GetActiveSlot(),
	          ActiveSlotBefore);
	TestEqual(TEXT("Failed collection spawns no pickup"), FailureWorld.CountPickups(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryDropAndNotificationTest,
	"NoTomorrow.Inventory.DropAndNotifications",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryDropAndNotificationTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld DropWorld;
	UNotoInventoryTestListener* Listener = NewObject<UNotoInventoryTestListener>();
	Listener->Initialize(DropWorld.Inventory);
	DropWorld.Inventory->OnInventoryChanged.AddDynamic(Listener, &UNotoInventoryTestListener::HandleInventoryChanged);
	DropWorld.Inventory->OnEquipmentChanged.AddDynamic(Listener, &UNotoInventoryTestListener::HandleEquipmentChanged);

	UNotoItemDefinition* Stack = MakeDefinition(TEXT("DropStack"), ENotoItemType::Tool, 5);
	const FGuid StackId = Add(*DropWorld.Inventory, *Stack, 5);
	TestEqual(TEXT("AddItem emits one inventory callback"), Listener->InventoryChangedCount, 1);
	TestEqual(TEXT("AddItem emits no equipment callback"), Listener->EquipmentChangedCount, 0);
	TestEqual(TEXT("AddItem callback observes final item count"), Listener->InventoryObservation.ItemCount, 1);
	TestEqual(TEXT("AddItem callback observes final quantity"), Listener->InventoryObservation.Quantity, 5);

	Listener->Reset();
	TestTrue(TEXT("Stack equips"), DropWorld.Inventory->EquipItem(StackId, ENotoEquipmentSlot::Tool5, false));
	TestEqual(TEXT("Simple equip emits no inventory callback"), Listener->InventoryChangedCount, 0);
	TestEqual(TEXT("Simple equip emits one equipment callback"), Listener->EquipmentChangedCount, 1);
	TestEqual(TEXT("Equip callback observes final equipment count"), Listener->EquipmentObservation.EquipmentCount, 1);
	TestEqual(TEXT("Equip callback observes final active slot"), Listener->EquipmentObservation.ActiveSlot,
	          ENotoEquipmentSlot::Tool5);

	Listener->Reset();
	TestTrue(TEXT("Partial drop succeeds"), DropWorld.Inventory->DropItem(StackId, 2));
	FNotoItemInstance RemainingItem;
	TestTrue(TEXT("Partially dropped stack remains"), DropWorld.Inventory->GetItem(StackId, RemainingItem));
	TestEqual(TEXT("Partial drop removes requested quantity"), RemainingItem.Quantity, 3);

	Listener->Reset();
	TestTrue(TEXT("Dropping the final quantity succeeds"), DropWorld.Inventory->DropItem(StackId, 3));
	TestFalse(TEXT("Final drop removes the item"), DropWorld.Inventory->GetItem(StackId, RemainingItem));
	TestFalse(
		TEXT("Final drop removes equipment reference"),
		DropWorld.Inventory->GetEquippedItem(ENotoEquipmentSlot::Tool5, RemainingItem));
	TestEqual(TEXT("Final drop repairs active slot"), DropWorld.Inventory->GetActiveSlot(), ENotoEquipmentSlot::None);
	TestEqual(TEXT("Full and partial drops create two pickups"), DropWorld.CountPickups(), 2);
	TestEqual(TEXT("Full equipped drop emits one inventory callback"), Listener->InventoryChangedCount, 1);
	TestEqual(TEXT("Full equipped drop emits one equipment callback"), Listener->EquipmentChangedCount, 1);
	TestEqual(TEXT("Full drop inventory callback observes no items"), Listener->InventoryObservation.ItemCount, 0);
	TestEqual(TEXT("Full drop inventory callback observes no equipment"), Listener->InventoryObservation.EquipmentCount,
	          0);
	TestEqual(TEXT("Full drop equipment callback observes no items"), Listener->EquipmentObservation.ItemCount, 0);
	TestEqual(
		TEXT("Full drop equipment callback observes repaired active slot"), Listener->EquipmentObservation.ActiveSlot,
		ENotoEquipmentSlot::None);

	FTestWorld NoPawnWorld(false);
	UNotoItemDefinition* NoPawnItem = MakeDefinition(TEXT("NoPawnItem"), ENotoItemType::Tool);
	const FGuid NoPawnItemId = Add(*NoPawnWorld.Inventory, *NoPawnItem, 1);
	TestFalse(TEXT("DropItem fails without a pawn"), NoPawnWorld.Inventory->DropItem(NoPawnItemId, 1));
	TestTrue(TEXT("DropItemAt remains available without a pawn"),
	         NoPawnWorld.Inventory->DropItemAt(NoPawnItemId, 1, FTransform(FVector(200.0f, 0.0f, 0.0f))));

	UNotoItemDefinition* OldWeapon = MakeDefinition(TEXT("OldNotificationWeapon"), ENotoItemType::MainWeapon);
	UNotoItemDefinition* NewWeapon = MakeDefinition(TEXT("NewNotificationWeapon"), ENotoItemType::MainWeapon);
	Collect(*DropWorld.Inventory, *OldWeapon);
	Listener->Reset();

	FGuid NewWeaponId;
	TestTrue(TEXT("Replacement collection succeeds"), DropWorld.Inventory->CollectItem(
		         NewWeapon, 1, -1, NewWeaponId));
	TestEqual(TEXT("Replacement emits one inventory callback"), Listener->InventoryChangedCount, 1);
	TestEqual(TEXT("Replacement emits one equipment callback"), Listener->EquipmentChangedCount, 1);
	TestEqual(TEXT("Replacement emits two callbacks"), Listener->CallbackOrder.Num(), 2);
	if (Listener->CallbackOrder.Num() == 2)
	{
		TestEqual(TEXT("Replacement broadcasts inventory first"), Listener->CallbackOrder[0], FName(TEXT("Inventory")));
		TestEqual(TEXT("Replacement broadcasts equipment second"), Listener->CallbackOrder[1],
		          FName(TEXT("Equipment")));
	}
	TestEqual(TEXT("Inventory callback observes final item count"), Listener->InventoryObservation.ItemCount, 1);
	TestEqual(TEXT("Inventory callback observes final equipment count"), Listener->InventoryObservation.EquipmentCount,
	          1);
	TestEqual(TEXT("Inventory callback observes final quantity"), Listener->InventoryObservation.Quantity, 1);
	TestEqual(TEXT("Inventory callback observes final active slot"), Listener->InventoryObservation.ActiveSlot,
	          ENotoEquipmentSlot::MainWeapon);
	TestEqual(TEXT("Equipment callback observes final item count"), Listener->EquipmentObservation.ItemCount, 1);
	TestEqual(TEXT("Equipment callback observes final equipment count"), Listener->EquipmentObservation.EquipmentCount,
	          1);
	TestEqual(TEXT("Equipment callback observes final quantity"), Listener->EquipmentObservation.Quantity, 1);
	TestEqual(TEXT("Equipment callback observes final active slot"), Listener->EquipmentObservation.ActiveSlot,
	          ENotoEquipmentSlot::MainWeapon);

	FTestWorld FailureWorld;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		UNotoItemDefinition* Tool = MakeDefinition(
			*FString::Printf(TEXT("NotificationBlockedTool%d"), Index),
			ENotoItemType::Tool,
			1,
			false);
		Collect(*FailureWorld.Inventory, *Tool);
	}
	UNotoInventoryTestListener* FailureListener = NewObject<UNotoInventoryTestListener>();
	FailureListener->Initialize(FailureWorld.Inventory);
	FailureWorld.Inventory->OnInventoryChanged.AddDynamic(FailureListener,
	                                                      &UNotoInventoryTestListener::HandleInventoryChanged);
	FailureWorld.Inventory->OnEquipmentChanged.AddDynamic(FailureListener,
	                                                      &UNotoInventoryTestListener::HandleEquipmentChanged);
	UNotoItemDefinition* BlockedTool = MakeDefinition(TEXT("NotificationBlockedReplacement"), ENotoItemType::Tool);
	FGuid FailedItemId;
	TestFalse(TEXT("Blocked collection fails"), FailureWorld.Inventory->CollectItem(
		          BlockedTool, 1, -1, FailedItemId));
	TestEqual(TEXT("Failed collection emits no inventory callback"), FailureListener->InventoryChangedCount, 0);
	TestEqual(TEXT("Failed collection emits no equipment callback"), FailureListener->EquipmentChangedCount, 0);
	TestEqual(TEXT("Failed collection records no callback order"), FailureListener->CallbackOrder.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoInventoryPickupAgreementTest,
	"NoTomorrow.Inventory.PickupAgreement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoInventoryPickupAgreementTest::RunTest(const FString& Parameters)
{
	using namespace NotoInventoryTests;

	FTestWorld SuccessWorld;
	UNotoItemDefinition* SuccessItem = MakeDefinition(TEXT("PickupSuccess"), ENotoItemType::Tool);
	ANotoItemPickup* SuccessPickup = SuccessWorld.World->SpawnActor<ANotoItemPickup>();
	SuccessPickup->InitializePickup(SuccessItem, 1, -1);
	TestTrue(TEXT("CanCollectItem reports success"), SuccessWorld.Inventory->CanCollectItem(SuccessItem, 1, -1));
	TestTrue(
		TEXT("Pickup CanInteract agrees with successful collection"),
		SuccessPickup->CanInteract_Implementation(SuccessWorld.Pawn));
	SuccessPickup->Interact_Implementation(SuccessWorld.Pawn);
	TestTrue(TEXT("Successful pickup interaction destroys the pickup"), SuccessPickup->IsActorBeingDestroyed());
	TestEqual(TEXT("Successful pickup transfers its item"), SuccessWorld.Inventory->GetTotalQuantity(SuccessItem), 1);

	FTestWorld DuplicateWeaponWorld;
	UNotoItemDefinition* DuplicateWeapon = MakeDefinition(
		TEXT("PickupDuplicateWeapon"), ENotoItemType::MainWeapon, 1, true, 6);
	FGuid DuplicateWeaponId;
	TestTrue(TEXT("Initial duplicate test weapon is collected"), DuplicateWeaponWorld.Inventory->CollectItem(
		         DuplicateWeapon, 1, 2, DuplicateWeaponId));
	ANotoItemPickup* DuplicateWeaponPickup = DuplicateWeaponWorld.World->SpawnActor<ANotoItemPickup>();
	DuplicateWeaponPickup->InitializePickup(DuplicateWeapon, 1, 4);
	TestTrue(
		TEXT("Partially loaded matching weapon can interact"),
		DuplicateWeaponPickup->CanInteract_Implementation(DuplicateWeaponWorld.Pawn));
	DuplicateWeaponPickup->Interact_Implementation(DuplicateWeaponWorld.Pawn);
	FNotoItemInstance RefilledWeapon;
	TestTrue(
		TEXT("Matching weapon remains equipped after ammo transfer"),
		DuplicateWeaponWorld.Inventory->GetItem(DuplicateWeaponId, RefilledWeapon));
	TestEqual(TEXT("Matching weapon receives the pickup rounds"), RefilledWeapon.LoadedAmmo, 6);
	TestTrue(TEXT("Consumed duplicate weapon pickup is destroyed"), DuplicateWeaponPickup->IsActorBeingDestroyed());

	ANotoItemPickup* InvalidQuantityWeaponPickup = DuplicateWeaponWorld.World->SpawnActor<ANotoItemPickup>();
	InvalidQuantityWeaponPickup->InitializePickup(DuplicateWeapon, 2, 4);
	TestFalse(
		TEXT("A non-stackable weapon pickup with quantity two cannot interact"),
		InvalidQuantityWeaponPickup->CanInteract_Implementation(DuplicateWeaponWorld.Pawn));
	FGuid InvalidQuantityItemId;
	TestFalse(
		TEXT("Collection rejects a non-stackable weapon quantity above one"),
		DuplicateWeaponWorld.Inventory->CollectItem(DuplicateWeapon, 2, 4, InvalidQuantityItemId));
	TestFalse(
		TEXT("Invalid-quantity weapon pickup remains in the world"),
		InvalidQuantityWeaponPickup->IsActorBeingDestroyed());

	ANotoItemPickup* FullWeaponPickup = DuplicateWeaponWorld.World->SpawnActor<ANotoItemPickup>();
	FullWeaponPickup->InitializePickup(DuplicateWeapon, 1, 1);
	TestFalse(
		TEXT("Full matching weapon cannot interact with another duplicate"),
		FullWeaponPickup->CanInteract_Implementation(DuplicateWeaponWorld.Pawn));
	FullWeaponPickup->Interact_Implementation(DuplicateWeaponWorld.Pawn);
	TestFalse(
		TEXT("Full matching weapon leaves the duplicate pickup in the world"),
		FullWeaponPickup->IsActorBeingDestroyed());

	FTestWorld FailureWorld;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		UNotoItemDefinition* Tool = MakeDefinition(
			*FString::Printf(TEXT("PickupBlockedTool%d"), Index),
			ENotoItemType::Tool,
			1,
			false);
		Collect(*FailureWorld.Inventory, *Tool);
	}
	UNotoItemDefinition* FailureItem = MakeDefinition(TEXT("PickupFailure"), ENotoItemType::Tool);
	ANotoItemPickup* FailurePickup = FailureWorld.World->SpawnActor<ANotoItemPickup>();
	FailurePickup->InitializePickup(FailureItem, 1, -1);
	TestFalse(
		TEXT("CanCollectItem reports blocked replacement"),
		FailureWorld.Inventory->CanCollectItem(FailureItem, 1, -1));
	TestFalse(
		TEXT("Pickup CanInteract agrees with blocked collection"),
		FailurePickup->CanInteract_Implementation(FailureWorld.Pawn));
	FGuid FailedItemId;
	TestFalse(
		TEXT("CollectItem agrees with blocked pickup"),
		FailureWorld.Inventory->CollectItem(FailureItem, 1, -1, FailedItemId));
	FailurePickup->Interact_Implementation(FailureWorld.Pawn);
	TestFalse(TEXT("Failed pickup remains in the world"), FailurePickup->IsActorBeingDestroyed());
	TestEqual(TEXT("Failed pickup does not mutate inventory"), FailureWorld.Inventory->GetTotalQuantity(FailureItem),
	          0);
	return true;
}

#endif
