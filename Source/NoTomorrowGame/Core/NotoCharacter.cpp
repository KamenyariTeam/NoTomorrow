// © 2025 Kamenyari. All rights reserved.


#include "NotoCharacter.h"


// Sets default values
ANotoCharacter::ANotoCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANotoCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANotoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ANotoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

