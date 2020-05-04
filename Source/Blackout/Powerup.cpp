// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerup.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APowerup::APowerup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->InitSphereRadius(TriggerRadius);
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &APowerup::OnTrigger);
	TriggerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	// TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	//if (Role != ROLE_Authority) {
	//	// Only the server should respond to triggers
	//	return;
	//}

	if (!GetVisible()) {
		// Do nothing if the powerup is not active
		return;
	}

	ABlackoutCharacter* character = dynamic_cast<ABlackoutCharacter*>(OtherActor);
	if (!character) {
		return;
	}

	SetVisible(false);
	UWorld* world = GetWorld();
	if (world) {
		world->GetTimerManager().SetTimer(respawnTimer, this, &APowerup::Respawn, RespawnTime, false);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Attempted to trigger powerup when not spawned. Tell Fred if you ever see this message."));
	}

	Powerup(character);
}

void APowerup::SetVisible(bool state) {
	isVisible = state;
	OnVisibilityUpdate();
}

void APowerup::OnVisibilityUpdate()
{
	this->SetActorHiddenInGame(!isVisible);
}

void APowerup::Respawn()
{
	SetVisible(true);
}

void APowerup::Powerup(ABlackoutCharacter* character) {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!void powerup. Tell Fred if you ever see this message."));
}

void APowerup::OnRep_Visibility()
{
	OnVisibilityUpdate();
}

void APowerup::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(APowerup, isVisible);
}
