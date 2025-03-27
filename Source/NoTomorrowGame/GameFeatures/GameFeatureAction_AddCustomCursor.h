// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_AddCustomCursor.generated.h"

struct FComponentRequestHandle;
class APlayerController;
class UUserWidget;

UCLASS(MinimalAPI, meta = (DisplayName = "Add Custom Cursor"))
class UGameFeatureAction_AddCustomCursor final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category = "Cursor", meta = (AssetBundles = "Client"))
	TMap<TEnumAsByte<EMouseCursor::Type>, TSoftClassPtr<UUserWidget>> CustomCursors;

private:
	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		TArray<TWeakObjectPtr<APlayerController>> ControllersAddedTo;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
    
	void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	void AddCursorWidgets(APlayerController* PlayerController, FPerContextData& ActiveData);
	void RemoveCursorWidgets(APlayerController* PlayerController, FPerContextData& ActiveData);
	void Reset(FPerContextData& ActiveData);
};