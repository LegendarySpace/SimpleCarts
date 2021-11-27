// Fill out your copyright notice in the Description page of Project Settings.


#include "GoCartMovementReplication.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

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
		ClientTick(DeltaTime);
	}
}

void UGoCartMovementReplication::ClientTick(const float DeltaTime)
{
	Client_TimeSinceUpdate += DeltaTime;

	if (Client_TimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (MovementComponent == nullptr) return;

	float LerpRatio = Client_TimeSinceUpdate / Client_TimeBetweenLastUpdates;

	auto Spline = CreateSpline();
	// Values not zero even with no input, erradic movements and velocitys
	InterpolateLocation(LerpRatio, Spline);
	InterpolateVelocity(LerpRatio, Spline);
	InterpolateRotation(LerpRatio);

}

float UGoCartMovementReplication::VelocityToDerivitive()
{
	return Client_TimeBetweenLastUpdates * 100;
}

FHermiteCubicSpline UGoCartMovementReplication::CreateSpline()
{
	FHermiteCubicSpline Spline;

	Spline.StartLocation = Client_StartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivitive = Client_StartVelocity * VelocityToDerivitive();
	Spline.TargetDerivitive = ServerState.Velocity * VelocityToDerivitive();

	return Spline;
}

void UGoCartMovementReplication::InterpolateLocation(const float LerpRatio, const FHermiteCubicSpline Spline)
{
	auto NewLocation = Spline.InterpolatePosition(LerpRatio);
	if (OffsetRoot != nullptr) OffsetRoot->SetWorldLocation(NewLocation);
}

void UGoCartMovementReplication::InterpolateVelocity(const float LerpRatio, const FHermiteCubicSpline Spline)
{
	auto NewVelocity = Spline.InterpolateDerivitive(LerpRatio) / VelocityToDerivitive();

	MovementComponent->SetVelocity(NewVelocity);
}

void UGoCartMovementReplication::InterpolateRotation(const float LerpRatio)
{
	FQuat StartRotation = Client_StartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();

	auto NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	if (OffsetRoot != nullptr) OffsetRoot->SetWorldRotation(NewRotation);
}

void UGoCartMovementReplication::UpdateServerState(FGoCartMove Move)
{
	ServerState.LastMove = Move;

	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool UGoCartMovementReplication::Server_SendMove_Validate(FGoCartMove Move)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;
	bool IsClientAhead = ProposedTime >= GetWorld()->GetTimeSeconds();
	if (IsClientAhead)
	{
		UE_LOG(LogTemp, Error, TEXT("Client Time is ahead of server"));
		return false;
	}
	if (!Move.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Received invalid move"));
		return false;
	}
	return true;
}

void UGoCartMovementReplication::Server_SendMove_Implementation(FGoCartMove Move)
{
	if (MovementComponent == nullptr) return;

	ClientSimulatedTime += Move.DeltaTime;
	UpdateServerState(Move);

	MovementComponent->SimulateMove(Move);
}

void UGoCartMovementReplication::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UGoCartMovementReplication::AutonomousProxy_OnRep_ServerState()
{
	if (GetOwner() == nullptr || MovementComponent == nullptr) return;

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoCartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoCartMovementReplication::SimulatedProxy_OnRep_ServerState()
{
	if (GetOwner() == nullptr || MovementComponent == nullptr) return;

	Client_TimeBetweenLastUpdates = Client_TimeSinceUpdate;
	Client_TimeSinceUpdate = 0;

	if (OffsetRoot != nullptr)
	{
		Client_StartTransform.SetLocation(OffsetRoot->GetComponentLocation());
		Client_StartTransform.SetRotation(OffsetRoot->GetComponentQuat());
	}

	Client_StartVelocity = MovementComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UGoCartMovementReplication::ClearAcknowledgedMoves(const FGoCartMove& LastMove)
{
	UnacknowledgedMoves.RemoveAll([&](FGoCartMove Move) { return Move.Timestamp <= LastMove.Timestamp; });
}

