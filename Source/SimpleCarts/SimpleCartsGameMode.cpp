// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleCartsGameMode.h"
#include "SimpleCartsPawn.h"
#include "SimpleCartsHud.h"

ASimpleCartsGameMode::ASimpleCartsGameMode()
{
	DefaultPawnClass = ASimpleCartsPawn::StaticClass();
	HUDClass = ASimpleCartsHud::StaticClass();
}
