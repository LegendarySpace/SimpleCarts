// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoCartMovementComponent.generated.h"


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

	bool IsValid()
	{
		return (FMath::Abs(Steering) <= 1 && FMath::Abs(Throttle) <= 1);
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLECARTS_API UGoCartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoCartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FGoCartMove CreateMove(float DeltaTime);

	void SimulateMove(const FGoCartMove& Move);

	FVector GetAirResistance();
	FVector GetRollingResistance();

	// Getters and Setters
	void SetThrottle(float InThrottle) { Throttle = FMath::Clamp<float>(InThrottle, -1, 1); }
	void SetSteering(float InSteering) { Steering = FMath::Clamp<float>(InSteering, -1, 1); }
	void SetVelocity(FVector InVelocity) { Velocity = InVelocity; }
	FVector GetVelocity() { return Velocity; }

	FGoCartMove LastMove;

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

	UPROPERTY(VisibleAnywhere)
	FVector Velocity;

	UPROPERTY(EditAnywhere)
	float Speed;

	UPROPERTY(EditAnywhere)
	float RotationRadians;

	float Steering;
	float Throttle;
};
