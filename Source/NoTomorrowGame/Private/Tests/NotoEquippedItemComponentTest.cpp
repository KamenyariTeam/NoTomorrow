// © 2025 Kamenyari. All rights reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Character/NotoEquippedItemComponent.h"

#include "Character/NotoCharacter.h"
#include "Character/NotoEquippedItemActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Inventory/NotoInventoryComponent.h"
#include "Inventory/NotoItemDefinition.h"
#include "Misc/AutomationTest.h"
#include "Player/NotoPlayerState.h"
#include "UObject/UnrealType.h"

namespace NotoEquippedItemTests
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

	UNotoItemDefinition* MakeItem(const TCHAR* Name, ENotoItemType ItemType)
	{
		UNotoItemDefinition* Definition = NewObject<UNotoItemDefinition>(GetTransientPackage(), FName(Name));
		SetEnumProperty(*Definition, TEXT("ItemType"), ItemType);
		FindPropertyChecked<FClassProperty>(Definition->GetClass(), TEXT("EquippedActorClass"))
			->SetPropertyValue_InContainer(Definition, ANotoEquippedItemActor::StaticClass());
		FindPropertyChecked<FNameProperty>(Definition->GetClass(), TEXT("EquippedSocketName"))
			->SetPropertyValue_InContainer(Definition, NAME_None);
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

			PlayerState = World->SpawnActor<ANotoPlayerState>();
			Character = World->SpawnActor<ANotoCharacter>();
			check(PlayerState && Character);
			Character->SetPlayerState(PlayerState);
		}

		~FTestWorld()
		{
			World->CleanupWorld();
			GEngine->DestroyWorldContext(World);
			World->ReleasePhysicsScene();
		}

		UWorld* World = nullptr;
		ANotoPlayerState* PlayerState = nullptr;
		ANotoCharacter* Character = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNotoEquippedItemActiveItemTest,
	"NoTomorrow.Equipment.ActiveItemPresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNotoEquippedItemActiveItemTest::RunTest(const FString& Parameters)
{
	using namespace NotoEquippedItemTests;

	FTestWorld TestWorld;
	UNotoInventoryComponent* Inventory = TestWorld.PlayerState->GetInventoryComponent();
	UNotoEquippedItemComponent* Presentation = TestWorld.Character->GetEquippedItemComponent();
	TestNotNull(TEXT("Character has a motion warping component"), TestWorld.Character->GetMotionWarpingComponent());
	TestNotNull(TEXT("Character has a presentation mesh component"), TestWorld.Character->GetPresentationMesh());
	TestEqual(TEXT("Unconfigured presentation falls back to the primary mesh"), TestWorld.Character->GetPresentationMesh(), TestWorld.Character->GetMesh());
	UNotoItemDefinition* Weapon = MakeItem(TEXT("PresentedWeapon"), ENotoItemType::MainWeapon);
	UNotoItemDefinition* Tool = MakeItem(TEXT("PresentedTool"), ENotoItemType::Tool);

	FGuid WeaponId;
	FGuid ToolId;
	TestTrue(TEXT("Weapon is added"), Inventory->AddItem(Weapon, 1, 0, WeaponId));
	TestTrue(TEXT("Tool is added"), Inventory->AddItem(Tool, 1, 0, ToolId));
	TestTrue(TEXT("Weapon equips"), Inventory->EquipItem(WeaponId, ENotoEquipmentSlot::MainWeapon, false));
	TestTrue(TEXT("Tool equips"), Inventory->EquipItem(ToolId, ENotoEquipmentSlot::Tool1, false));
	TestTrue(TEXT("Weapon becomes active"), Inventory->SetActiveSlot(ENotoEquipmentSlot::MainWeapon));

	Presentation->InitializeWithInventory(Inventory);
	TestEqual(TEXT("Active weapon identity drives presentation"), Presentation->GetDisplayedItemInstanceId(), WeaponId);
	ANotoEquippedItemActor* DisplayedWeapon = Presentation->GetDisplayedActor();
	TestNotNull(TEXT("Weapon presentation actor is spawned"), DisplayedWeapon);
	if (DisplayedWeapon)
	{
		TestEqual(TEXT("Weapon actor receives its definition"), DisplayedWeapon->GetItemDefinition(), Weapon);
		TestEqual(TEXT("Weapon actor receives its item identity"), DisplayedWeapon->GetItemInstanceId(), WeaponId);
	}

	TestTrue(TEXT("Tool becomes active"), Inventory->SetActiveSlot(ENotoEquipmentSlot::Tool1));
	TestEqual(TEXT("Equipment event updates tool identity"), Presentation->GetDisplayedItemInstanceId(), ToolId);
	ANotoEquippedItemActor* DisplayedTool = Presentation->GetDisplayedActor();
	TestNotNull(TEXT("Tool presentation actor is spawned"), DisplayedTool);
	if (DisplayedTool)
	{
		TestNotEqual(TEXT("Changing active items replaces the actor"), DisplayedTool, DisplayedWeapon);
		TestEqual(TEXT("Tool actor receives its definition"), DisplayedTool->GetItemDefinition(), Tool);
	}

	TestTrue(TEXT("Hands can be cleared"), Inventory->SetActiveSlot(ENotoEquipmentSlot::None));
	TestNull(TEXT("Cleared presentation destroys its actor"), Presentation->GetDisplayedActor());
	return true;
}

#endif
