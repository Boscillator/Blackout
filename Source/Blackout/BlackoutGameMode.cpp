// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlackoutGameMode.h"
#include "BlackoutHUD.h"
#include "BlackoutCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

ABlackoutGameMode::ABlackoutGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ABlackoutHUD::StaticClass();
}

void ABlackoutGameMode::RespawnPlayer(ABlackoutCharacter* pawn) {
	pawn->SetCurrentHealth(pawn->MaxHealth);
	TArray<AActor*> spawn_points;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), spawn_points);
	int spawn_index = FMath::RandRange(0, spawn_points.Num() - 1);	// Who makes a random number generate right inclusive?
	AActor* spawn = spawn_points[spawn_index];

	pawn->SetActorLocation(spawn->GetActorLocation());
	pawn->SetAmmo(pawn->ClipSize);
}