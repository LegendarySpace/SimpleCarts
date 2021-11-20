// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCart.h"

#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGoCart::AGoCart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

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

void AGoCart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoCart, ServerState);
	DOREPLIFETIME(AGoCart, Steering);
	DOREPLIFETIME(AGoCart, Throttle);
}


// Called when the game starts or when spawned
void AGoCart::BeginPlay()
{
	Super::BeginPlay();
	NetUpdateFrequency = 2;
}

// Called every frame
void AGoCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		// Create Move
		FGoCartMove Move;
		Move.Steering = Steering;
		Move.Throttle = Throttle;
		Move.DeltaTime = DeltaTime;

		// Send Move to server
		Server_SendMove(Move);
	}

	// Calculate Acceleration
	FVector Force = MaxDrivingForce * Throttle * GetActorForwardVector();

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	// Apply Acceleration
	Velocity += Acceleration * DeltaTime;

	// Update rotation and position
	UpdateRotation(DeltaTime, Steering);
	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		ServerState.Transform = GetActorTransform();
		ServerState.Velocity = Velocity;
	}

	Speed = Velocity.Size();
}

void AGoCart::OnRep_ServerState()
{
	UE_LOG(LogTemp, Warning, TEXT("Updating State"));
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
}

void AGoCart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoCart::MoveRight(float Value)
{
	Steering = Value;
}

bool AGoCart::Server_SendMove_Validate(FGoCartMove Move)
{
	return (FMath::Abs(Move.Steering) <= 1 && FMath::Abs(Move.Throttle) <= 1);
}

void AGoCart::Server_SendMove_Implementation(FGoCartMove Move)
{
	// Should add to stack of moves
	Steering = Move.Steering;
	Throttle = Move.Throttle;
}

FVector AGoCart::GetAirResistance()
{
	return - Velocity.SizeSquared() * DragCoefficient * Velocity.GetSafeNormal();
}

FVector AGoCart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;

	// NF = Mass * -Gravity
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void AGoCart::UpdateLocationFromVelocity(float DeltaTime)
{
	// Calculate Resistances
	FVector Translation = Velocity * DeltaTime * 100;
	

	FHitResult* Hit = new FHitResult;
	AddActorWorldOffset(Translation, true, Hit);

	if (Hit->IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

FQuat AGoCart::UpdateRotation(float DeltaTime, float SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * 10 * DeltaTime;
	float RotationAngle = DeltaLocation / TurningRadius * SteeringThrow;
	RotationRadians = RotationAngle * DeltaTime;

	FQuat Rot(GetActorUpVector(), RotationRadians);
	AddActorWorldRotation(Rot);
	Velocity = Rot.RotateVector(Velocity);
	return Rot;
}

