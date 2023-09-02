// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "RDCPlayerState.generated.h"

class URDCCharacterAbilityComponent;

/**
 * 
 */
UCLASS()
class RDCHMLNCASE_API ARDCPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ARDCPlayerState();

	// Implement IAbilitySystemInterface
	 virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


protected:
	UPROPERTY()
	TObjectPtr<URDCCharacterAbilityComponent> AbilitySystemComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
