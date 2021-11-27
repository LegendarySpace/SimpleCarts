// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCart.h"

#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AGoCart::AGoCart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoCartMovementComponent>(TEXT("Movement Component"));
	MovementReplication = CreateDefaultSubobject<UGoCartMovementReplication>(TEXT("Movement Replication"));

}
// Called to bind functionality to input
void AGoCart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoCart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoCart::MoveRight);
}


// Called when the game starts or when spawned
void AGoCart::BeginPlay()
{
	Super::BeginPlay();
	NetUpdateFrequency = 1;
	SetReplicateMovement(false);
}

// Called every frame
void AGoCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGoCart::MoveForward(float Value)
{
	if (MovementComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Movement Component fail"));
		return;
	}
	MovementComponent->SetThrottle(Value);
}

void AGoCart::MoveRight(float Value)
{
	if (MovementComponent == nullptr) return;
	MovementComponent->SetSteering(Value);
}

