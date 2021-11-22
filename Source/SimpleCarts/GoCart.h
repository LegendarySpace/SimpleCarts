// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "GoCartMovementComponent.h"
#include "GoCartMovementReplication.h"

#include "GoCart.generated.h"


UCLASS()
class SIMPLECARTS_API AGoCart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoCart();

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UGoCartMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Replication")
	class UGoCartMovementReplication* MovementReplication;

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
};

