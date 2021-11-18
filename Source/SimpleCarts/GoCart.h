// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoCart.generated.h"

UCLASS()
class SIMPLECARTS_API AGoCart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoCart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	FVector CalculateAcceleration();

	FVector GetAirResistance();
	FVector GetRollingResistance();

private:
	void UpdateLocationFromVelocity(float DeltaTime);

	FQuat UpdateRotation(float DeltaTime);

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
	float TurningRadius = 2;

	UPROPERTY(EditAnywhere)
	FVector Velocity = FVector(0.0);

	UPROPERTY(EditAnywhere)
	float Speed;

	UPROPERTY(EditAnywhere)
	float Steering;

	UPROPERTY(EditAnywhere)
	float RotationRadians;

	float Throttle;
};

