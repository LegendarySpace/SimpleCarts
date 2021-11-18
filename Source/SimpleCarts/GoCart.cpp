// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCart.h"

#include "Engine/World.h"

// Sets default values
AGoCart::AGoCart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoCart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Velocity += CalculateAcceleration() * DeltaTime;
	Speed = Velocity.Size();

	UpdateRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);
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

void AGoCart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoCart::MoveRight(float Value)
{
	Steering = Value;
}

FVector AGoCart::CalculateAcceleration()
{
	FVector Force = MaxDrivingForce * Throttle * GetActorForwardVector();

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	return Acceleration;
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

FQuat AGoCart::UpdateRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * 10 * DeltaTime;
	float RotationAngle = DeltaLocation / TurningRadius * Steering;
	RotationRadians = RotationAngle * DeltaTime;

	// Need to flip rotation when throttle is negative
	// If (forward vector + NormVelocity).Size is < 1 going reverse
	//if ((GetActorForwardVector() + Velocity.GetSafeNormal()).Size() < 1) RotationRadians *= -1;

	FQuat Rot(GetActorUpVector(), RotationRadians);
	AddActorWorldRotation(Rot);
	Velocity = Rot.RotateVector(Velocity);
	return Rot;
}

