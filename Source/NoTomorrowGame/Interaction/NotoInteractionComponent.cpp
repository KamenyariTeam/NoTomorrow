// © 2025 Kamenyari. All rights reserved.

#include "Interaction/NotoInteractionComponent.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Player/NotoPlayerController.h"
#include "Interaction/NotoInteractable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoInteractionComponent)

UNotoInteractionComponent::UNotoInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNotoInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	InteractionRange = NewObject<USphereComponent>(GetOwner(), TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(GetOwner()->GetRootComponent());
	InteractionRange->InitSphereRadius(InteractionRadius);
	InteractionRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionRange->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionRange->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionRange->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleInteractionRangeBeginOverlap);
	InteractionRange->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleInteractionRangeEndOverlap);
	InteractionRange->RegisterComponent();
	Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandlePawnControllerChanged);
	RefreshInteractionEnabled();
}

void UNotoInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandlePawnControllerChanged);
	}

	SetSelectedInteractable(nullptr);
	NearbyInteractables.Empty();
	Super::EndPlay(EndPlayReason);
}

void UNotoInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshSelection();
}

void UNotoInteractionComponent::TryInteract()
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()); Pawn && IsValid(SelectedInteractable) && INotoInteractable::Execute_CanInteract(SelectedInteractable, Pawn))
	{
		INotoInteractable::Execute_Interact(SelectedInteractable, Pawn);
	}
}

AActor* UNotoInteractionComponent::GetSelectedInteractable() const
{
	return SelectedInteractable;
}

void UNotoInteractionComponent::HandleInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) && OtherActor->Implements<UNotoInteractable>())
	{
		NearbyInteractables.Add(OtherActor);
	}
}

void UNotoInteractionComponent::HandleInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	NearbyInteractables.Remove(OtherActor);
}

void UNotoInteractionComponent::HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	RefreshInteractionEnabled();
}

void UNotoInteractionComponent::RefreshInteractionEnabled()
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const bool bEnabled = Pawn && Pawn->IsLocallyControlled();
	SetComponentTickEnabled(bEnabled);
	if (InteractionRange)
	{
		InteractionRange->SetGenerateOverlapEvents(bEnabled);
	}

	if (!bEnabled)
	{
		SetSelectedInteractable(nullptr);
		NearbyInteractables.Empty();
	}
}

void UNotoInteractionComponent::RefreshSelection()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	const ANotoPlayerController* PlayerController = Pawn ? Cast<ANotoPlayerController>(Pawn->GetController()) : nullptr;
	if (!Pawn || !PlayerController)
	{
		SetSelectedInteractable(nullptr);
		return;
	}

	FVector2D CursorPosition;
	if (!PlayerController->GetGameplayReticlePosition(CursorPosition))
	{
		SetSelectedInteractable(nullptr);
		return;
	}

	AActor* BestInteractable = nullptr;
	double BestDistanceSquared = TNumericLimits<double>::Max();
	for (auto It = NearbyInteractables.CreateIterator(); It; ++It)
	{
		AActor* Candidate = It->Get();
		if (!IsValid(Candidate))
		{
			It.RemoveCurrent();
			continue;
		}

		if (!INotoInteractable::Execute_CanInteract(Candidate, Pawn))
		{
			continue;
		}

		FVector2D CandidateScreenPosition;
		if (PlayerController->ProjectWorldLocationToScreen(Candidate->GetActorLocation(), CandidateScreenPosition, true))
		{
			if (const double DistanceSquared = FVector2D::DistSquared(CursorPosition, CandidateScreenPosition); DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				BestInteractable = Candidate;
			}
		}
	}

	SetSelectedInteractable(BestInteractable);
}

void UNotoInteractionComponent::SetSelectedInteractable(AActor* NewSelectedInteractable)
{
	if (SelectedInteractable == NewSelectedInteractable)
	{
		return;
	}

	if (IsValid(SelectedInteractable))
	{
		INotoInteractable::Execute_SetInteractionHighlighted(SelectedInteractable, false);
	}

	SelectedInteractable = NewSelectedInteractable;
	if (IsValid(SelectedInteractable))
	{
		INotoInteractable::Execute_SetInteractionHighlighted(SelectedInteractable, true);
	}
}
