// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlackoutHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ABlackoutHUD::ABlackoutHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> GameOverTexObj(TEXT("/Game/UI/GameOver"));
	GameOverTex = GameOverTexObj.Object;
}


void ABlackoutHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y + 20.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );

	if (drawGameOver) {
		const FVector2D gameOverDrawPosition((Center.X - GameOverTex->GetSizeX() / 2), (Center.Y - GameOverTex->GetSizeY() / 2));
		FCanvasTileItem gameOverTileItem(gameOverDrawPosition, GameOverTex->Resource, FLinearColor::White);
		gameOverTileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(gameOverTileItem);
	}
}

void ABlackoutHUD::DrawGameOver()
{
	
	UWorld* world = GetWorld();
	if (world) {
		drawGameOver = true;
		world->GetTimerManager().SetTimer(gameOverTimerHandle, this, &ABlackoutHUD::ResumePlayAfterGameOver, gameOverMessageTime, false);
	} else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Attempted to game over when not spawned. Tell Fred if you ever see this message."));
	}
}


void ABlackoutHUD::ResumePlayAfterGameOver()
{
	drawGameOver = false;
}