// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCartMovementComponent.h"

// Sets default values for this component's properties
UGoCartMovementComponent::UGoCartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoCartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoCartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		LastMove = CreateMove(DeltaTime);

		SimulateMove(LastMove);
	}

	Speed = Velocity.Size();
}

FGoCartMove UGoCartMovementComponent::CreateMove(float DeltaTime)
{
	FGoCartMove Move;
	Move.Steering = Steering;
	Move.Throttle = Throttle;
	Move.DeltaTime = DeltaTime;
	Move.Timestamp = GetWorld()->GetTimeSeconds();

	return Move;
}

void UGoCartMovementComponent::SimulateMove(const FGoCartMove& Move)
{
	// Calculate Acceleration
	FVector Force = MaxDrivingForce * Move.Throttle * GetOwner()->GetActorForwardVector();

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	// Apply Acceleration
	Velocity += Acceleration * Move.DeltaTime;

	// Update rotation and position
	UpdateRotation(Move.DeltaTime, Move.Steering);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FVector UGoCartMovementComponent::GetAirResistance()
{
	return -Velocity.SizeSquared() * DragCoefficient * Velocity.GetSafeNormal();
}

FVector UGoCartMovementComponent::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;

	// NF = Mass * -Gravity
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void UGoCartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	// Calculate Resistances
	FVector Translation = Velocity * DeltaTime * 100;


	FHitResult* Hit = new FHitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, Hit);

	if (Hit->IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

FQuat UGoCartMovementComponent::UpdateRotation(float DeltaTime, float SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * 10 * DeltaTime;
	float RotationAngle = DeltaLocation / TurningRadius * SteeringThrow;
	RotationRadians = RotationAngle * DeltaTime;

	FQuat Rot(GetOwner()->GetActorUpVector(), RotationRadians);
	GetOwner()->AddActorWorldRotation(Rot);
	Velocity = Rot.RotateVector(Velocity);
	return Rot;
}

