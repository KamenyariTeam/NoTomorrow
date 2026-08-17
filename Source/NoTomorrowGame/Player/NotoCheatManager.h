// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameFramework/CheatManager.h"
#include "NotoCheatManager.generated.h"

class UNotoInventoryComponent;

/** Project developer commands. Unreal does not instantiate cheat managers in shipping builds. */
UCLASS()
class NOTOMORROWGAME_API UNotoCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	virtual void InitCheatManager() override;
	virtual void BeginDestroy() override;

	void HandlePlayerStateChanged();

	UFUNCTION(Exec)
	void NotoInventoryDump() const;

	UFUNCTION(Exec)
	void NotoInventoryDebug();

	UFUNCTION(Exec)
	void NotoInventoryGive(const FString& ItemDefinitionPath, int32 Quantity = 1, int32 LoadedAmmo = -1);

	UFUNCTION(Exec)
	void NotoInventoryDropActive(int32 Quantity = 1);

	UFUNCTION(Exec)
	void NotoInventorySetActiveSlot(int32 SlotValue);

private:
	UFUNCTION()
	void HandleInventoryChanged();

	UNotoInventoryComponent* GetInventoryComponent() const;
	void RefreshInventoryBinding();
	void UnbindInventory();

	TWeakObjectPtr<UNotoInventoryComponent> BoundInventory;
	bool bInventoryDebugEnabled = false;
};
