// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoCart.generated.h"


USTRUCT()
struct FGoCartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Steering;

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Timestamp;
};

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

UCLASS()
class SIMPLECARTS_API AGoCart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoCart();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	void MoveForward(float Value);

	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoCartMove Move);

	FVector GetAirResistance();
	FVector GetRollingResistance();

private:
	void UpdateLocationFromVelocity(float DeltaTime);

	FQuat UpdateRotation(float DeltaTime, float SteeringThrow);

private:
	// Mass in kg
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// Full throttle force (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// MaxDrivingForce / TopSpeed^2
	// Aerodynamic measure of go kart (0 == Perfect)
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = .015;

	// Size of turning radius in meters
	UPROPERTY(EditAnywhere)
	float TurningRadius = 1;

	UPROPERTY(VisibleAnywhere, Replicated)
	FVector Velocity;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoCartState ServerState;

	UFUNCTION()
	void OnRep_ServerState();

	UPROPERTY(EditAnywhere)
	float Speed;

	UPROPERTY(EditAnywhere, Replicated)
	float Steering;

	UPROPERTY(EditAnywhere)
	float RotationRadians;

	UPROPERTY(EditAnywhere, Replicated)
	float Throttle;
};

