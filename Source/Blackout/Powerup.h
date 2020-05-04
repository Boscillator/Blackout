// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Powerup.generated.h"

UCLASS()
class BLACKOUT_API APowerup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Gameplay")
	float TriggerRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Gameplay")
	float RespawnTime = 4.f;

	UPROPERTY()
	class USphereComponent* TriggerSphere;

	UFUNCTION()
	void OnTrigger(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Respawn();

	void SetVisible(bool state);
	bool GetVisible() { return isVisible; }


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FTimerHandle respawnTimer;
	bool isVisible = true;
};
