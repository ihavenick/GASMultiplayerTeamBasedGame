// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "Data/EAbilityID.h"
#include "GameFramework/Character.h"
#include "Data/ETeam.h"
#include "Interfaces/RDCAbilitySystemInterface.h"
#include "RDCBaseCharacter.generated.h"

class UHealthComponent;
class UWidgetComponent;
class URDCCharacterAbilityComponent;


UCLASS(config=Game)
class ARDCBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IRDCAbilitySystemInterface
{
	GENERATED_BODY()
	
	FTimerHandle FTH_Team;
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;
	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;
	
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;
	

public:
	ARDCBaseCharacter();
	
	bool ASCInputBound = false;
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Target)
	UArrowComponent* TargetPoint;

	TObjectPtr<URDCCharacterAbilityComponent> AbilitySystemComponent;

	//Replicated control rotation for aimofset
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	FRotator ReplicatedControlRotation;
	
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<AProjectileActor> ProjectileClass;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


	// Input to ability system
	void JumpAbility(const FInputActionValue& Value);
	void JumpAbilityStop(const FInputActionValue& Value);
	void ShootAbility(const FInputActionValue& Value);
	void ShootAbilityStop(const FInputActionValue& Value);
	void SendLocalInputToAbilitySystem(bool bIsPressed, EAbilityInputID AbilityInputID);

	// Ability interface

	virtual void SendInputtoAbility_Implementation(const EAbilityInputID AbilityID, bool bPressed) override;

	virtual void GetProjectileVariables_Implementation(FRotator& SpawnRotation, FVector& SpawnLocation, TSubclassOf<AProjectileActor>& ProjectileClassOut) override;

	virtual void PlayReplicatedMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate, FName StartSectionName) override;

	virtual void StopReplicatedMontage_Implementation(UAnimMontage* AnimMontage) override;
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	void RemoveCharacterAbilities();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RDC | Components")
	TObjectPtr<UHealthComponent> HealthComp;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RDC | Abilities")
	TArray<TSubclassOf<class URDCGameplayAbilityBase>> CharacterAbilities;
	

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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	// End of APawn interface

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "RDC | Varibles")
	ETeams Team;
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category="RDC | Varibles")
	bool bDead;
	UFUNCTION(BlueprintCallable)
	void GetControllerRotationReplicated();
	
	UFUNCTION(Category = "RDC | Varibles")
	float GetHealthPercent() const;
	UFUNCTION(BlueprintCallable, Category = "RDC | Abilities")
	virtual int32 GetAbilityLevel(EAbilityInputID AbilityID) const;
	void AddCharacterAbilities();
	
	UFUNCTION(NetMulticast, Reliable)
	void Ragdoll();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayMontage(UAnimMontage* MontageToPlay, float InPlayRate,FName StartSectionName);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMontage(UAnimMontage* MontageToPlay, float InPlayRate, FName StartSectionName);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopMontage(UAnimMontage* MontageToStop);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopMontage(UAnimMontage* MontageToStop);
	
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RDC | Components")
	UWidgetComponent *HealthBar{nullptr};
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

