// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlackoutCharacter.h"
#include "BlackoutGameMode.generated.h"

UCLASS(minimalapi)
class ABlackoutGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABlackoutGameMode();
	void RespawnPlayer(ABlackoutCharacter* pawn);
};



