// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "SimpleCartsHud.generated.h"


UCLASS(config = Game)
class ASimpleCartsHud : public AHUD
{
	GENERATED_BODY()

public:
	ASimpleCartsHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
