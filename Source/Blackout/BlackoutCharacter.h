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

	/** The light which lights up the immediate surroundings around the player. Only visible to the owner. */
	UPROPERTY(VisibleAnywhere, Category = "Switch Components")
	class UPointLightComponent* PersonalLight;

public:
	ABlackoutCharacter();

protected:
	/** Called when the game launches */
	virtual void BeginPlay();

	/** Called once every tick */
	void Tick(float deltaTime) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	float LookSpeedScaler;

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

	/**
	* Called when the pause key is pressed
	*/
	UFUNCTION(BlueprintCallable)
	void Pause();

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

	void Turn(float rate);
	void LookUp(float val);


	
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

	/** Getter for Current Ammo. */
	UFUNCTION(BlueprintPure, Category = "Ammo")
	FORCEINLINE int GetAmmo() const { return Ammo; }

	/** Getter for the clip size */
	UFUNCTION(BlueprintPure, Category = "Ammo")
	FORCEINLINE int GetClipSize() const { return ClipSize; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(int healthValue);

	/** Setter for the current amount of ammo. */
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void SetAmmo(int ammoValule);

	/** Number of seconds between shots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float fireRate;

	/** Sound to play when the user dies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* DeathSound;

	/** Sound to play when the user takes a step */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* FootStep;

	/** Number of seconds between footstep sounds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float footStepRate;

	/** Mimimum velocity the player must be moving to trigger footsteps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float footStepMinVelocity;

	/** Called once every `footStepRate` seconds. */
	UFUNCTION()
	void OnFootstep();

	/** Called by UGameplayStatics::ApplyPointDamage */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Called when the user should die. Only runs on the server. */
	UFUNCTION(Server, Reliable)
	void Die();

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	int MaxHealth;

	/** The max amount of ammo the user can have. */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int ClipSize;

	/** If Health <= NotifyAtHealth then set the PersonalLight's color to LowHealthColor */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	int NotifyAtHealth;

	/** The default color of PersonalLight */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	FLinearColor FullHeathColor;

	/** The color of PersonalLight when the user runs low on health. */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	FLinearColor LowHealthColor;

	/** Sound to play when the player runs out of ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* OutOfAmmoSound;

	UFUNCTION(NetMulticast, Reliable)
	void OutOfAmmoAnimation();



protected:
	UFUNCTION(Server, Reliable)
	void DoFire();

	UFUNCTION(NetMulticast, Reliable)
	void DieAnimation(const FString& name);

	UFUNCTION(NetMulticast, Reliable)
	void DoFireAnimation();

	/** Called when the amount of health changes */
	void OnHealthUpdate();

	/** Called when the amount of ammo changes */
	void OnAmmoUpdate();

private:
	/** Records the amount of time, in seconds, since the user last shoot. Used for fire delay */
	float timeSinceLastShot;

	/** True if the pause menu is shown, and the player shouldn't respond to inputs */
	bool paused;

	/** Used by unreal to call OnFootstep at regular intervals */
	FTimerHandle footstepHandler;

	/** Current amount of ammo the player has */
	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int Ammo;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	int CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Changes have been made to the current ammo */
	UFUNCTION()
	void OnRep_Ammo();
};

