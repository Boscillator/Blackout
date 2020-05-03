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

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	// Create Local Flashlight
	PointLight1 = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight1"));
	PointLight1->SetIsReplicated(false);
	PointLight1->Intensity = 50.f;
	PointLight1->SetAttenuationRadius(400.f);
	PointLight1->SetupAttachment(RootComponent);
	PointLight1->bVisible = true;

	// Health
	MaxHealth = 2;
	CurrentHealth = MaxHealth;
	NotifyAtHealth = 1;
	FullHeathColor = FLinearColor::White;
	LowHealthColor = FLinearColor::Red;
}

void ABlackoutCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}


	// Disable every else's flashlight
	if (!IsLocallyControlled()) {
		PointLight1->SetHiddenInGame(true);

	}

	UWorld* world = GetWorld();
	if (world) {
		world->GetTimerManager().SetTimer(footstepHandler, this, &ABlackoutCharacter::OnFootstep, footStepRate, true);
	}
	
	OnHealthUpdate();
}

void ABlackoutCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(ABlackoutCharacter, CurrentHealth);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABlackoutCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlackoutCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ABlackoutCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlackoutCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlackoutCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABlackoutCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABlackoutCharacter::LookUpAtRate);
}

void ABlackoutCharacter::OnFire()
{

	if (timeSinceLastShot < fireRate) {
		// Do nothing if you have shot recently.
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Can't Shoot"));
		return;
	}

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		DoFire();
		timeSinceLastShot = 0;
	}

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Here"));



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
		if (bUsingMotionControllers)
		{
			const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
			const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();

			FActorSpawnParameters spawnParameters;
			spawnParameters.Instigator = Instigator;
			spawnParameters.Owner = this;

			World->SpawnActor<ABlackoutProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, spawnParameters);
		}
		else
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

			DoFireAnimation();
		}
	}
}

void ABlackoutCharacter::DoFireAnimation_Implementation() {
	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}

void ABlackoutCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ABlackoutCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ABlackoutCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ABlackoutCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

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
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABlackoutCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ABlackoutCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ABlackoutCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ABlackoutCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ABlackoutCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}


float ABlackoutCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	// FString deathMessage = FString::Printf(TEXT("Intesgator %x. Me %x"), EventInstigator, GetController());
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, deathMessage);
	if (GetController() != EventInstigator) {
		int damageApplied = CurrentHealth - DamageTaken;
		SetCurrentHealth(damageApplied);
		return damageApplied;
	}
	return 0;
}

void ABlackoutCharacter::Die_Implementation() {
	AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();
	AActor* spawnPoint = gameMode->FindPlayerStart(this->GetController());


	SetActorLocation(spawnPoint->GetActorLocation(), false);
	CurrentHealth = MaxHealth;
	OnHealthUpdate();

	APlayerController* playerController = dynamic_cast<APlayerController*>(GetController());
	if (playerController) {
		DieAnimation(playerController->GetName());
	}
	
}

void ABlackoutCharacter::DieAnimation_Implementation(const FString& name) {
	if (IsLocallyControlled())
	{
		FString deathMessage = FString::Printf(TEXT("You have been killed."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
	}

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

void ABlackoutCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		if (CurrentHealth > NotifyAtHealth) {
			PointLight1->SetLightColor(FullHeathColor);
		}
		else {
			PointLight1->SetLightColor(LowHealthColor);
		}

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
		if (healthValue < MaxHealth) {
			CurrentHealth = healthValue;
		}
		OnHealthUpdate();
	}
}