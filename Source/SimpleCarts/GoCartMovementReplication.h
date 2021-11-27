// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GoCartMovementComponent.h"

#include "GoCartMovementReplication.generated.h"


USTRUCT()
struct FGoCartState
{
	GENERATED_BODY()

		UPROPERTY()
		FGoCartMove LastMove;

	UPROPERTY()
		FVector Velocity;

	UPROPERTY()
		FTransform Transform;
};

struct FHermiteCubicSpline
{

	FVector StartLocation, TargetLocation, StartDerivitive, TargetDerivitive;

	FVector InterpolatePosition(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivitive,  TargetLocation, TargetDerivitive, LerpRatio);
	}

	FVector InterpolateDerivitive(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivitive, TargetLocation, TargetDerivitive, LerpRatio);
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLECARTS_API UGoCartMovementReplication : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoCartMovementReplication();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ClientTick(float DeltaTime);

	float VelocityToDerivitive();
	FHermiteCubicSpline CreateSpline();
	void InterpolateLocation(const float LerpRatio, const FHermiteCubicSpline Spline);
	void InterpolateVelocity(const float LerpRatio, const FHermiteCubicSpline Spline);
	void InterpolateRotation(const float LerpRatio);

	void UpdateServerState(FGoCartMove Move);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(const FGoCartMove Move);

	UFUNCTION()
	void OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();

	void ClearAcknowledgedMoves(const FGoCartMove& LastMove);

private:
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoCartState ServerState;

	TArray<FGoCartMove> UnacknowledgedMoves;

	float Client_TimeSinceUpdate;
	float Client_TimeBetweenLastUpdates;

	FTransform Client_StartTransform;
	FVector Client_StartVelocity;

	float ClientSimulatedTime;

	UGoCartMovementComponent* MovementComponent;

	UPROPERTY()
	USceneComponent* OffsetRoot;

	UFUNCTION(BlueprintCallable)
	void SetOffsetRoot(USceneComponent* Root) { OffsetRoot = Root; }

};
