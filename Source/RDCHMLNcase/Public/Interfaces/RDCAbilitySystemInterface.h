// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RDCAbilitySystemInterface.generated.h"

enum class EAbilityID : uint8;
// This class does not need to be modified.
UINTERFACE()
class URDCAbilitySystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RDCHMLNCASE_API IRDCAbilitySystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//sends input to ability system
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SendInputtoAbility(const EAbilityInputID AbilityID, bool bPressed);
	//Gets projectile spawn variables
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetProjectileVariables(FRotator& SpawnRotation, FVector& SpawnLocation, TSubclassOf<class AProjectileActor>& ProjectileClass);
	//plays replicated montage
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayReplicatedMontage(UAnimMontage* MontageToPlay, float InPlayRate, FName StartSectionName);
	// stops montage replicated
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopReplicatedMontage(UAnimMontage* MontageToStop);
};
