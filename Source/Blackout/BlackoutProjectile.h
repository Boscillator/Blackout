// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "Components/PointLightComponent.h"
#include "BlackoutProjectile.generated.h"

UCLASS(config=Game)
class ABlackoutProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	class USphereComponent* CollisionComp;

	UPROPERTY(EditAnywhere, Category = Projectile)
	float Speed = 10000.f;

	UPROPERTY(VisibleAnywhere, Category = Visual)
	class UPointLightComponent* Light;


	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	bool dissipating = false;

public:
	ABlackoutProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* DissipateSound;

	UPROPERTY(EditAnywhere, Category = Visual)
	FLinearColor DefaultLightingColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = Visual)
	float DefaultLightAttenuationRadius = 2500;

	UPROPERTY(EditAnywhere, Category = Visual)
	float DefaultLightIntensity = 50000;

	UFUNCTION(BlueprintCallable, Category = Visual)
	void SetLightColor(FLinearColor color);
};

