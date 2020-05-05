// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlackoutCharacter.h"
#include "BlackoutProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "BlackoutGameMode.h"
#include "BlackoutHud.h"
#include "GameFramework/GameModeBase.h"


DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ABlackoutCharacter

ABlackoutCharacter::ABlackoutCharacter()
{

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	// GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABlackoutCharacter::OnHit);
	

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create Local Flashlight
	PersonalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PersonalLight"));
	PersonalLight->SetIsReplicated(false);
	PersonalLight->Intensity = 50.f;
	PersonalLight->SetAttenuationRadius(400.f);
	PersonalLight->SetupAttachment(RootComponent);
	PersonalLight->bVisible = true;

	// Set default health
	MaxHealth = 2;
	CurrentHealth = MaxHealth;
	NotifyAtHealth = 1;
	FullHeathColor = FLinearColor::White;
	LowHealthColor = FLinearColor::Red;

	// Set default ammo
	ClipSize = 6;
}

void ABlackoutCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));


	// Disable every else's flashlight
	if (!IsLocallyControlled()) {
		// The code here only runs on the computer which owns this actor (the computer of the player controlling this character)
		PersonalLight->SetHiddenInGame(true);
	}

	// Set up a time manager.
	UWorld* world = GetWorld();
	if (world) {
		// For some reason GetWorld() can return null, so we had better check for it so we don't get a seg-fault. We have to do that alot in this code
		world->GetTimerManager().SetTimer(footstepHandler, this, &ABlackoutCharacter::OnFootstep, footStepRate, true);
	}
	
	// Start player with 6 clips
	SetAmmo(ClipSize);

	// Update health once so the lights update
	OnHealthUpdate();
}

void ABlackoutCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This tells Unreal that we want to synchronize these values. 
	DOREPLIFETIME(ABlackoutCharacter, CurrentHealth);
	DOREPLIFETIME(ABlackoutCharacter, Ammo);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABlackoutCharacter::Pause()
{
	if (APlayerController* playerController = dynamic_cast<APlayerController*>(GetController())) {
		if (ABlackoutHUD* hud = dynamic_cast<ABlackoutHUD*>(playerController->GetHUD())) {
			hud->TogglePaused();
			paused = !paused;
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Must use ABlackoutHud in pause. Tell Fred if you ever see this message."));
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Tried to pause for non-player pawn. Tell Fred if you ever see this message."));
	}
}

void ABlackoutCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// All auto generated stuff that just works

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlackoutCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlackoutCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlackoutCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ABlackoutCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABlackoutCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlackoutCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABlackoutCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ABlackoutCharacter::Pause);
}

void ABlackoutCharacter::OnFire()
{
	if (paused) {
		return;
	}

	if (GetAmmo() <= 0) {
		// No ammo, can't shoot
		return;
	}

	if (timeSinceLastShot < fireRate) {
		// Do nothing if you have shot recently.
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Can't Shoot"));
		return;
	}

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		// We want the actual shot to run on the server, so we call an RPC to run it there.
		DoFire();
		timeSinceLastShot = 0;
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ABlackoutCharacter::DoFire_Implementation()
{
	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		ActorSpawnParams.Instigator = Instigator;
		ActorSpawnParams.Owner = this;

		// spawn the projectile at the muzzle
		World->SpawnActor<ABlackoutProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

		// Decrease the players ammo by one
		SetAmmo(GetAmmo() - 1);

		// Call DoFireAnimation on all clients to play the sound and what-not
		DoFireAnimation();
	}
}

void ABlackoutCharacter::DoFireAnimation_Implementation() {
	// Called on all clients

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}


void ABlackoutCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ABlackoutCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ABlackoutCharacter::TurnAtRate(float Rate)
{
	if (paused) {
		return;
	}

	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABlackoutCharacter::LookUpAtRate(float Rate)
{
	if (paused) {
		return;
	}

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABlackoutCharacter::Turn(float val)
{
	if (paused) {
		return;
	}

	AddControllerYawInput(val);
}

void ABlackoutCharacter::LookUp(float val)
{
	if (paused) {
		return;
	}

	AddControllerPitchInput(val);
}


float ABlackoutCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	// Don't allow self-damage
	if (GetController() != EventInstigator) {
		// Decrement and apply health
		int damageApplied = CurrentHealth - DamageTaken;
		SetCurrentHealth(damageApplied);
		return damageApplied;
	}
	return 0;
}

// Called on the server when the character dies
void ABlackoutCharacter::Die_Implementation() {

	// Game mode is respawnable for handling respawning, get it.
	ABlackoutGameMode* gameMode = dynamic_cast<ABlackoutGameMode*>(GetWorld()->GetAuthGameMode());
	if (gameMode) {
		gameMode->RespawnPlayer(this);
	}
	else {
		// Something when wrong
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!ABlackoutCharacter must be used with ABlackoutGameMode or subclass. Tell Fred if you ever see this message."));
	}

	// For legacy reasons, we need to get the play controller, I wounder if this will change
	APlayerController* playerController = dynamic_cast<APlayerController*>(GetController());
	if (playerController) {
		DieAnimation(playerController->GetPlayerNetworkAddress());
	}
	else {
		// This shouldn't happen. Did someone try to implement AI?
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Tried to kill a non-player pawn. Tell Fred if you ever see this message."));
	}
	
}

void ABlackoutCharacter::DieAnimation_Implementation(const FString& name) {
	// Only display the death message on the controlling player's computer
	if (IsLocallyControlled())
	{
		// Make sure both the controller and hud are correct, otherwise something is wrong
		if (APlayerController* playerController = dynamic_cast<APlayerController*>(GetController())) {
			if (ABlackoutHUD* hud = dynamic_cast<ABlackoutHUD*>(playerController->GetHUD())) {
				hud->DrawGameOver();
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Must use ABlackoutHud. Tell Fred if you ever see this message."));
			}
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("!!!Tried to display death for non-player pawn. Tell Fred if you ever see this message."));
		}
	}

	// Display death message
	FString deathMessage = FString::Printf(TEXT("%s has been blacked out."), *name);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);

	if(DeathSound != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
}

void ABlackoutCharacter::Tick(float deltaTime) {
	Super::Tick(deltaTime);
	timeSinceLastShot += deltaTime;
}

void ABlackoutCharacter::OnFootstep() {
	// Called at regular intervals to play footsteps.

	if (GetVelocity().Size() > footStepMinVelocity && GetCharacterMovement()->IsWalking()) {
		if (FootStep != NULL) {
			UGameplayStatics::PlaySoundAtLocation(this, FootStep, GetActorLocation());
		}
	}
}

void ABlackoutCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void ABlackoutCharacter::OnRep_Ammo()
{
	OnAmmoUpdate();
}

void ABlackoutCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		// Update the lighting
		if (CurrentHealth > NotifyAtHealth) {
			PersonalLight->SetLightColor(FullHeathColor);
		}
		else {
			PersonalLight->SetLightColor(LowHealthColor);
		}

		// Die if you should
		if (CurrentHealth <= 0)
		{
			Die();
		}
	}
}

void ABlackoutCharacter::SetCurrentHealth(int healthValue)
{
	if (Role == ROLE_Authority)
	{
		if (healthValue <= MaxHealth) {
			CurrentHealth = healthValue;
		}
		OnHealthUpdate();
	}
}

void ABlackoutCharacter::SetAmmo(int ammoValue)
{
	if (Role == ROLE_Authority)
	{
		if (0 <= ammoValue && ammoValue <= ClipSize) {
			Ammo = ammoValue;
		}
		OnAmmoUpdate();
	}
}

void ABlackoutCharacter::OnAmmoUpdate() {}
