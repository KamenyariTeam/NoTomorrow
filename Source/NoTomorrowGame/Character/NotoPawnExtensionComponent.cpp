// © 2025 Kamenyari. All rights reserved.

#include "NotoPawnExtensionComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "NotoPawnData.h"
#include "Development/NotoGameplayTags.h"
#include "Development/NotoLogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPawnExtensionComponent)

const FName UNotoPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

UNotoPawnExtensionComponent::UNotoPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
}

void UNotoPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNotoPawnExtensionComponent, PawnData);
}

void UNotoPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf(Pawn != nullptr, TEXT("NotoPawnExtensionComponent must be added to a Pawn actor (Owner: %s)."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UNotoPawnExtensionComponent::StaticClass(), Components);
	ensureAlwaysMsgf(Components.Num() == 1, TEXT("Only one NotoPawnExtensionComponent should be on Pawn %s."), *GetNameSafe(GetOwner()));

	// Register with the initialization system.
	RegisterInitStateFeature();
}

void UNotoPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();
	// Listen for initialization state changes.
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	ensure(TryToChangeInitState(NotoGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UNotoPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void UNotoPawnExtensionComponent::SetPawnData(const UNotoPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();

	// Only the authority should set pawn data.
	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogNoto, Error, TEXT("Attempting to set PawnData [%s] on Pawn [%s] that already has PawnData [%s]."),
			*GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;
	Pawn->ForceNetUpdate();
	CheckDefaultInitialization();
}

void UNotoPawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UNotoPawnExtensionComponent::HandleControllerChanged()
{
	// Simply refresh initialization when the controller changes.
	CheckDefaultInitialization();
}

void UNotoPawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UNotoPawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UNotoPawnExtensionComponent::CheckDefaultInitialization()
{
	// Call any implementers first.
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = { 
		NotoGameplayTags::InitState_Spawned, 
		NotoGameplayTags::InitState_DataAvailable, 
		NotoGameplayTags::InitState_DataInitialized, 
		NotoGameplayTags::InitState_GameplayReady 
	};

	// Progress through the initialization states.
	ContinueInitStateChain(StateChain);
}

bool UNotoPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == NotoGameplayTags::InitState_Spawned)
	{
		// Valid pawn indicates spawned.
		return (Pawn != nullptr);
	}
	if (CurrentState == NotoGameplayTags::InitState_Spawned && DesiredState == NotoGameplayTags::InitState_DataAvailable)
	{
		// Require PawnData.
		if (!PawnData)
		{
			return false;
		}

		bool bHasAuthority = Pawn->HasAuthority();
		bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Ensure pawn is possessed.
			if (!GetController<AController>())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == NotoGameplayTags::InitState_DataAvailable && DesiredState == NotoGameplayTags::InitState_DataInitialized)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn, NotoGameplayTags::InitState_DataAvailable);
	}
	else if (CurrentState == NotoGameplayTags::InitState_DataInitialized && DesiredState == NotoGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UNotoPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// No extra processing is needed for this simplified version.
}

void UNotoPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has reached DataAvailable, try progressing our initialization.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == NotoGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

