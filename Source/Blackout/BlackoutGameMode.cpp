// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlackoutGameMode.h"
#include "BlackoutHUD.h"
#include "BlackoutCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABlackoutGameMode::ABlackoutGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ABlackoutHUD::StaticClass();
}
