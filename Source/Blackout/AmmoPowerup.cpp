// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPowerup.h"

void AAmmoPowerup::Powerup(ABlackoutCharacter* character) {
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("Give Ammo"));
	character->SetAmmo(character->ClipSize);
}