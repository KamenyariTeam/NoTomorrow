// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatureAction_AddCustomCursor.h"

#include "Blueprint/UserWidget.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#define LOCTEXT_NAMESPACE "GameFeatures"

void UGameFeatureAction_AddCustomCursor::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
    FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
    if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
        !ensure(ActiveData.ControllersAddedTo.IsEmpty()))
    {
        Reset(ActiveData);
    }
    Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddCustomCursor::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
    Super::OnGameFeatureDeactivating(Context);

    FPerContextData* ActiveData = ContextData.Find(Context);
    if (ensure(ActiveData))
    {
        Reset(*ActiveData);
    }
}

void UGameFeatureAction_AddCustomCursor::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
    UWorld* World = WorldContext.World();
    UGameInstance* GameInstance = WorldContext.OwningGameInstance;
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

    if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
    {
        if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
        {
            UGameFrameworkComponentManager::FExtensionHandlerDelegate HandlerDelegate =
                UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleControllerExtension, ChangeContext);
            TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
                ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), HandlerDelegate);

            ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
        }
    }
}

void UGameFeatureAction_AddCustomCursor::HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
    APlayerController* PC = CastChecked<APlayerController>(Actor);
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

    if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || 
        EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
    {
        RemoveCursorWidgets(PC, ActiveData);
    }
    else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded)
    {
        AddCursorWidgets(PC, ActiveData);
    }
}

void UGameFeatureAction_AddCustomCursor::AddCursorWidgets(APlayerController* PlayerController, FPerContextData& ActiveData)
{
    if (!IsValid(PlayerController) || ActiveData.ControllersAddedTo.Contains(PlayerController))
    {
        return;
    }

    for (const auto& CursorPair : CustomCursors)
    {
        if (UClass* WidgetClass = CursorPair.Value.LoadSynchronous())
        {
            if (UUserWidget* CursorWidget = CreateWidget(PlayerController, WidgetClass))
            {
                PlayerController->SetMouseCursorWidget(CursorPair.Key, CursorWidget);
            }
        }
    }

    ActiveData.ControllersAddedTo.Add(PlayerController);
}

void UGameFeatureAction_AddCustomCursor::RemoveCursorWidgets(APlayerController* PlayerController, FPerContextData& ActiveData)
{
    if (PlayerController && ActiveData.ControllersAddedTo.Contains(PlayerController))
    {
        for (const auto& CursorPair : CustomCursors)
        {
            PlayerController->SetMouseCursorWidget(CursorPair.Key, nullptr);
        }
        ActiveData.ControllersAddedTo.Remove(PlayerController);
    }
}

void UGameFeatureAction_AddCustomCursor::Reset(FPerContextData& ActiveData)
{
    ActiveData.ExtensionRequestHandles.Empty();

    while (!ActiveData.ControllersAddedTo.IsEmpty())
    {
        TWeakObjectPtr<APlayerController> ControllerPtr = ActiveData.ControllersAddedTo.Top();
        if (ControllerPtr.IsValid())
        {
            RemoveCursorWidgets(ControllerPtr.Get(), ActiveData);
        }
        else
        {
            ActiveData.ControllersAddedTo.Pop();
        }
    }
}

#undef LOCTEXT_NAMESPACE
