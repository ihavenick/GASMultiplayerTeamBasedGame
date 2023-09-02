// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RDCBaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Character/Abilities/RDCGameplayAbilityBase.h"
#include "Character/Components/RDCHealthComponent.h"
#include "Character/Components/RDCCharacterAbilityComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "Game/RDCPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/RDCHealthBarWidget.h"



//////////////////////////////////////////////////////////////////////////
// ARDCBaseCharacter

ARDCBaseCharacter::ARDCBaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	
	bDead = false; // Set dead state to false on default
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	

	//////////////////////////////////// Health Bar ////////////////////////////////////
	HealthBar = CreateDefaultSubobject<UWidgetComponent>(FName("HealthBar"));
	static ConstructorHelpers::FClassFinder<UUserWidget> MenuWidgetClassFinder(TEXT("/Game/Core/UI/UI_HealthBar"));   // Doesnt need to hardcode but i want to use bp as low as i can
	HealthBar->SetWidgetClass(MenuWidgetClassFinder.Class);
	HealthBar->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	HealthBar->SetRelativeTransform(FTransform(FQuat(0,0,0,0),FVector(0,0,100),FVector(0,0.235f,0.0325f)));

	//////////////////////////////////// Health Comp ////////////////////////////////////
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;


	//////////////////////////////////// Target Point ////////////////////////////////////
	///(( Used to get the direction of the shoot ability ))
    TargetPoint = CreateDefaultSubobject<UArrowComponent>(FName("TargetArrow"));
	TargetPoint->SetupAttachment(GetMesh(),FName("ShootLocation"));
	TargetPoint->SetHiddenInGame(true);
    TargetPoint->SetRelativeRotation(FRotator(90,0,0),false,nullptr);
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	bReplicates = true; // Replicate this actor
	bAlwaysRelevant = true; // Always relevant for replication
}

//////////////////////////////////////////////////////////////////////////

void ARDCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	HealthBar->InitWidget(); // Init the health bar widget
	if (const auto HealBarUserWidget = Cast<UHealthBarWidget>(HealthBar->GetUserWidgetObject())) // If we successfully cast the widget to UHealthBarWidget
	{
		HealBarUserWidget->SetOwner(this); // Set the owner of the widget to this (to check teams)
		HealBarUserWidget->GetHealthBar()->PercentDelegate.BindUFunction(this, FName("GetHealthPercent")); // Bind the health bar percent delegate to the GetHealthPercent function (to automaticly update the health bar)    //TODO This works like tick and need to be changed, Updating this in TakeDamage is better
	}
	HealthComp->OnHealthChanged.AddUniqueDynamic(this, &ARDCBaseCharacter::OnHealthChanged); // Bind the OnHealthChanged event pf component to the OnHealthChanged function of this class
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARDCBaseCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARDCBaseCharacter::Move);

		//Jump Ability
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ARDCBaseCharacter::JumpAbility);             //Probably there is better solution
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARDCBaseCharacter::JumpAbilityStop);

		//Shoot Ability
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ARDCBaseCharacter::ShootAbility);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ARDCBaseCharacter::ShootAbilityStop);
		
		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARDCBaseCharacter::Look);
		
	}

}

UAbilitySystemComponent * ARDCBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get(); // Return the ASC
}


// Server only
void ARDCBaseCharacter::PossessedBy(AController * NewController)  // Inits ASC on server
{
	Super::PossessedBy(NewController);

	if (ARDCPlayerState* PS = GetPlayerState<ARDCPlayerState>()) // If we successfully cast the player state to ARDCPlayerState
	{
		AbilitySystemComponent = Cast<URDCCharacterAbilityComponent>(PS->GetAbilitySystemComponent()); // Cast the ASC of the player state to URDCCharacterAbilityComponent and set it to the ASC of this class
		
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this); // Init the ASC Actor Info

		AddCharacterAbilities(); // Add the abilities of this character
	}
}

// Client only
void ARDCBaseCharacter::OnRep_PlayerState()  // Inits ASC on client
{
	Super::OnRep_PlayerState();

	if (ARDCPlayerState* PS = GetPlayerState<ARDCPlayerState>())
	{
		AbilitySystemComponent = Cast<URDCCharacterAbilityComponent>(PS->GetAbilitySystemComponent()); 
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
		
	}
}


void ARDCBaseCharacter::RemoveCharacterAbilities()  // Remove the abilities of this character (mostly on death)
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent || !AbilitySystemComponent->bIsAbilitiesGiven)
	{
		return;
	}

	// Remove any abilities added from a previous call. This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if ((Spec.SourceObject == this) && CharacterAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}

	// Do in two passes so the removal happens after we have the full list
	for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	}

	AbilitySystemComponent->bIsAbilitiesGiven = false; // Set the flag to false (default)
}


void ARDCBaseCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARDCBaseCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARDCBaseCharacter::JumpAbility(const FInputActionValue& Value)
{
	SendLocalInputToAbilitySystem(true, EAbilityInputID::Jump);
}

void ARDCBaseCharacter::JumpAbilityStop(const FInputActionValue& Value)
{
	SendLocalInputToAbilitySystem(false, EAbilityInputID::Jump);
}

void ARDCBaseCharacter::ShootAbility(const FInputActionValue& Value)
{
	SendLocalInputToAbilitySystem(true, EAbilityInputID::Shoot);
}

void ARDCBaseCharacter::ShootAbilityStop(const FInputActionValue& Value)
{
	SendLocalInputToAbilitySystem(false, EAbilityInputID::Shoot);
}

void ARDCBaseCharacter::SendInputtoAbility_Implementation(const EAbilityInputID AbilityID, bool bPressed)
{
	SendLocalInputToAbilitySystem(bPressed, AbilityID);
}

void ARDCBaseCharacter::SendLocalInputToAbilitySystem(bool bIsPressed, const EAbilityInputID AbilityInputID)
{
	if(!AbilitySystemComponent)
		return;

	bIsPressed ? AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(AbilityInputID)) : AbilitySystemComponent->AbilityLocalInputReleased(static_cast<int32>(AbilityInputID)); // Send the input to the ASC
}

void ARDCBaseCharacter::GetProjectileVariables_Implementation(FRotator& SpawnRotation, FVector& SpawnLocation,
	TSubclassOf<AProjectileActor>& ProjectileClassOut)
{
	TargetPoint->GetSocketWorldLocationAndRotation("",SpawnLocation,SpawnRotation);
	SpawnRotation = GetFollowCamera()->GetComponentRotation();
	ProjectileClassOut = ProjectileClass;
}

void ARDCBaseCharacter::PlayReplicatedMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate,
	FName StartSectionName)
{
	if (!HasAuthority())
		ServerPlayMontage(MontageToPlay,InPlayRate,StartSectionName);

	if(!GetCurrentMontage())
		PlayAnimMontage(MontageToPlay,InPlayRate,StartSectionName);
}

void ARDCBaseCharacter::StopReplicatedMontage_Implementation(UAnimMontage* AnimMontage)
{
	if (HasAuthority())
		MulticastStopMontage(AnimMontage);
	else
		ServerStopMontage(AnimMontage);
}

void ARDCBaseCharacter::ServerPlayMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate,
                                                         FName StartSectionName)
{
	MulticastPlayMontage(MontageToPlay,InPlayRate,StartSectionName);
}

bool ARDCBaseCharacter::ServerPlayMontage_Validate(UAnimMontage* MontageToPlay, float InPlayRate,
	FName StartSectionName)
{
	return true;
}

void ARDCBaseCharacter::MulticastPlayMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate,
	FName StartSectionName)
{
	PlayAnimMontage(MontageToPlay,InPlayRate,StartSectionName);
}

void ARDCBaseCharacter::ServerStopMontage_Implementation(UAnimMontage* MontageToStop)
{
	MulticastStopMontage(MontageToStop);
}

bool ARDCBaseCharacter::ServerStopMontage_Validate(UAnimMontage* MontageToStop)
{
	return true;
}

void ARDCBaseCharacter::MulticastStopMontage_Implementation(UAnimMontage* MontageToStop)
{
	StopAnimMontage(MontageToStop);
}

void ARDCBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ARDCBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	
	
	if (GetLocalRole() < ROLE_Authority)  //Only work on local replication (Turns Health bars to face the camera)
	{
		if(HealthBar)
		{
			FRotator CameraRotation = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0)->GetCameraRotation();
			CameraRotation.Yaw-=180;
			HealthBar->SetWorldRotation(CameraRotation);
		}
	}
	
	GetControllerRotationReplicated(); //Replicates the controller rotation to the server
}

float ARDCBaseCharacter::GetHealthPercent() const
{
	return HealthComp->GetHealth() / 100; //Returns the health percentage
}

int32 ARDCBaseCharacter::GetAbilityLevel(EAbilityInputID AbilityID) const
{
	return 1; //Returns the ability level (not implemented)
}


void ARDCBaseCharacter::AddCharacterAbilities()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent || AbilitySystemComponent->bIsAbilitiesGiven)
	{
		return;
	}

	for (TSubclassOf<URDCGameplayAbilityBase>& StartupAbility : CharacterAbilities) //iterate list and give default abilities
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(StartupAbility, GetAbilityLevel(StartupAbility.GetDefaultObject()->AbilityID),
			                     static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this));
	}

	AbilitySystemComponent->bIsAbilitiesGiven = true; //Set the flag to true so we don't give abilities again
}

void ARDCBaseCharacter::Ragdoll_Implementation()  //Death function but i misnamed it
{
	RemoveCharacterAbilities();
	
	USkeletalMeshComponent* mesh = GetMesh();
	UCharacterMovementComponent* cm = GetCharacterMovement();
	UCapsuleComponent* UCC = GetCapsuleComponent();

	UCC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UCC->SetEnableGravity(false);

	cm->SetMovementMode(MOVE_None);

	CameraBoom->AttachToComponent(mesh, FAttachmentTransformRules::KeepRelativeTransform, FName("head"));

	mesh->SetCollisionObjectType(ECC_PhysicsBody);

	mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	mesh->SetAllBodiesSimulatePhysics(true);


	TArray<UStaticMeshComponent*> Components;
	
	GetComponents<UStaticMeshComponent>(Components);
	for( int32 i=0; i<Components.Num(); i++ )
	{
		UStaticMeshComponent* StaticMeshComponent = Components[i];
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ARDCBaseCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
                                        class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDead)  //Death check
	{
		bDead = true;
		GetMovementComponent()->StopMovementImmediately(); 
		Ragdoll();
	}
}

void ARDCBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARDCBaseCharacter, Team);
	DOREPLIFETIME_CONDITION(ARDCBaseCharacter, ReplicatedControlRotation, COND_SkipOwner);
	DOREPLIFETIME(ARDCBaseCharacter, bDead);
}

void ARDCBaseCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARDCBaseCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARDCBaseCharacter::GetControllerRotationReplicated()
{
	if (HasAuthority()||IsLocallyControlled())
	{
		ReplicatedControlRotation = GetControlRotation();
	}
}