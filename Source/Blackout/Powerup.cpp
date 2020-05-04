// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerup.h"

// Sets default values
APowerup::APowerup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->InitSphereRadius(TriggerRadius);
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &APowerup::OnTrigger);
	RootComponent = TriggerSphere;

}

// Called when the game starts or when spawned
void APowerup::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APowerup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APowerup::OnTrigger(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetVisible()) {
		// Do nothing if the powerup is not active
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("Power Up Noises"));
	SetVisible(false);

	UWorld* world = GetWorld();
	if (world) {
		world->GetTimerManager().SetTimer(respawnTimer, this, &APowerup::Respawn, RespawnTime, false);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Attempted to trigger powerup when not spawned. Tell Fred if you ever see this message."));
	}
}

void APowerup::SetVisible(bool state) {
	this->SetActorHiddenInGame(!state);
	isVisible = state;
}

void APowerup::Respawn()
{
	SetVisible(true);
}


