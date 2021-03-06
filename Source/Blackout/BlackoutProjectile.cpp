// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlackoutProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BlackoutCharacter.h"


ABlackoutProjectile::ABlackoutProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	if (Role == ROLE_Authority) {
		CollisionComp->OnComponentHit.AddDynamic(this, &ABlackoutProjectile::OnHit);		// set up a notification for when this component hits something blocking
	}

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Create Local Flashlight
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetIsReplicated(true);
	Light->Intensity = DefaultLightIntensity;
	Light->SetAttenuationRadius(DefaultLightAttenuationRadius);
	Light->SetupAttachment(RootComponent);
	Light->SetLightColor(DefaultLightingColor);
	Light->bVisible = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 1.0f;
}

void ABlackoutProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		// OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}
	if (OtherActor != NULL && this->Instigator) {
		UGameplayStatics::ApplyPointDamage(OtherActor, 1, NormalImpulse, Hit, Instigator->Controller, this, UDamageType::StaticClass());
	}

	// Only play the sound if it exists, and it's not already playing
	if (DissipateSound != NULL && !dissipating) {
		UGameplayStatics::PlaySoundAtLocation(this, DissipateSound, GetActorLocation());
	}
	dissipating = true;

	if (dynamic_cast<ABlackoutCharacter*>(OtherActor)) {
		Destroy();
	}
	else {
		SetLifeSpan(.2f);
	}
}

void ABlackoutProjectile::SetLightColor(FLinearColor color)
{
	Light->SetLightColor(color);
}
