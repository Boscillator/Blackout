// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "BlackoutCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class ABlackoutCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Switch Components")
	class UPointLightComponent* PointLight1;

public:
	ABlackoutCharacter();

protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ABlackoutProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE int GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE int GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Ammo")
	FORCEINLINE int GetAmmo() const { return Ammo; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(int healthValue);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void SetAmmo(int ammoValule);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float fireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Footstep)
	class USoundBase* FootStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Footstep)
	float footStepRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Footstep)
	float footStepMinVelocity;

	UFUNCTION()
	void OnFootstep();

	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(Server, Reliable)
	void Die();

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	int MaxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int ClipSize = 6;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	int NotifyAtHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	FLinearColor FullHeathColor;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	FLinearColor LowHealthColor;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	int CurrentHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int Ammo;


	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_Ammo();


	UFUNCTION(NetMulticast, Reliable)
	void DieAnimation(const FString& name);

	void Tick(float deltaTime) override;

protected:
	UFUNCTION(Server, Reliable)
	void DoFire();

	UFUNCTION(NetMulticast, Reliable)
	void DoFireAnimation();

	void OnHealthUpdate();
	void OnAmmoUpdate();

private:
	float timeSinceLastShot;
	FTimerHandle footstepHandler;
};

