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


	void ClearAcknowledgedMoves(const FGoCartMove& LastMove);

	void UpdateServerState(FGoCartMove Move);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoCartMove Move);

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoCartState ServerState;

	UFUNCTION()
	void OnRep_ServerState();

private:
	TArray<FGoCartMove> UnacknowledgedMoves;

	UGoCartMovementComponent* MovementComponent;
};
