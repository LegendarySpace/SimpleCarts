// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCart.h"

#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

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
}


// Called when the game starts or when spawned
void AGoCart::BeginPlay()
{
	Super::BeginPlay();
	NetUpdateFrequency = 1;
}

// Called every frame
void AGoCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FString str;
	switch (GetLocalRole())
	{
	case ROLE_Authority:
		str = "Authority";
		break;
	case ROLE_AutonomousProxy:
		str = "Autonomous";
		break;
	case ROLE_SimulatedProxy:
		str = "Simulated";
		break;
	default:
		break;
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), str, this, FColor::White, DeltaTime);

	// Client and in control of pawn
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		FGoCartMove Move = CreateMove(DeltaTime);

		UnacknowledgedMoves.Add(Move);

		// Send Move to server
		Server_SendMove(Move);

		SimulateMove(Move);
	}

	// Server and in control of pawn
	if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
	{
		FGoCartMove Move = CreateMove(DeltaTime);

		Server_SendMove(Move);
	}

	// Not in control of pawn
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
		// ServerState.LastMove and ServerState.Velocity are empty
		// Only Server Transform seems to be updated

		//UE_LOG(LogTemp, Warning, TEXT("Proxy throttle is: %d, and Steering is: %d"), ServerState.LastMove.Throttle, ServerState.LastMove.Steering);
		//UE_LOG(LogTemp, Warning, TEXT("Proxy Speed is: %d"), ServerState.Velocity.Size());
	}

	Speed = Velocity.Size();
}

FGoCartMove AGoCart::CreateMove(float DeltaTime)
{
	FGoCartMove Move;
	Move.Steering = Steering;
	Move.Throttle = Throttle;
	Move.DeltaTime = DeltaTime;
	Move.Timestamp = GetWorld()->GetTimeSeconds();
	
	return Move;
}

void AGoCart::SimulateMove(const FGoCartMove& Move)
{
	// Calculate Acceleration
	FVector Force = MaxDrivingForce * Move.Throttle * GetActorForwardVector();

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	// Apply Acceleration
	Velocity += Acceleration * Move.DeltaTime;

	// Update rotation and position
	UpdateRotation(Move.DeltaTime, Move.Steering);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

void AGoCart::ClearAcknowledgedMoves(const FGoCartMove& LastMove)
{
	UnacknowledgedMoves.RemoveAll([&](FGoCartMove Move) { return Move.Timestamp <= LastMove.Timestamp; });
}

void AGoCart::OnRep_ServerState()
{
	// Client Recieving Updated State
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	if (GetLocalRole() == ROLE_SimulatedProxy) UE_LOG(LogTemp, Warning, TEXT("OnRep_State Proxy Speed is: %d"), Velocity.Size());
	ClearAcknowledgedMoves(ServerState.LastMove);

	// Simulate all moves
	for (const FGoCartMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
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
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
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

