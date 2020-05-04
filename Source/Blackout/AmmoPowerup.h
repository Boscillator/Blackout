// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Powerup.h"
#include "AmmoPowerup.generated.h"

/**
 * 
 */
UCLASS()
class BLACKOUT_API AAmmoPowerup : public APowerup
{
	GENERATED_BODY()
protected:
    void Powerup(ABlackoutCharacter* character) override;
};
