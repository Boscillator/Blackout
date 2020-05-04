// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutHUD.generated.h"

UCLASS()
class ABlackoutHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABlackoutHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	void DrawGameOver();

protected:
	void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere)
	class UUserWidget* CurrentWidget;

private:

	void ResumePlayAfterGameOver();

	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;
	class UTexture2D* GameOverTex;

	float gameOverMessageTime = 1.5f;

	bool drawGameOver = false;
	FTimerHandle gameOverTimerHandle;

};

