// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCartMovementReplication.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGoCartMovementReplication::UGoCartMovementReplication()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);

	// ...
}


void UGoCartMovementReplication::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoCartMovementReplication, ServerState);
}



// Called when the game starts
void UGoCartMovementReplication::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoCartMovementComponent>();
	
}


// Called every frame
void UGoCartMovementReplication::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) return;

	FGoCartMove LastMove = MovementComponent->LastMove;

	// Client and in control of pawn
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);

		// Send Move to server
		Server_SendMove(LastMove);
	}

	// Server and in control of pawn
	if (GetOwnerRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		UpdateServerState(LastMove);
	}

	// Not in control of pawn
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
		// ServerState is empty
	}
}

void UGoCartMovementReplication::ClearAcknowledgedMoves(const FGoCartMove& LastMove)
{
	UnacknowledgedMoves.RemoveAll([&](FGoCartMove Move) { return Move.Timestamp <= LastMove.Timestamp; });
}

void UGoCartMovementReplication::UpdateServerState(FGoCartMove Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoCartMovementReplication::OnRep_ServerState()
{
	if (GetOwner() == nullptr || MovementComponent == nullptr) return;

	// Client Recieving Updated State
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy) UE_LOG(LogTemp, Warning, TEXT("OnRep_State Proxy Speed is: %d"), MovementComponent->GetVelocity().Size());
	ClearAcknowledgedMoves(ServerState.LastMove);

	// Simulate all moves
	for (const FGoCartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}


bool UGoCartMovementReplication::Server_SendMove_Validate(FGoCartMove Move)
{
	return (FMath::Abs(Move.Steering) <= 1 && FMath::Abs(Move.Throttle) <= 1);
}

void UGoCartMovementReplication::Server_SendMove_Implementation(FGoCartMove Move)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

