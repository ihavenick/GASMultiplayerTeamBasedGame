// Ata Çetin case


#include "Game/RDCPlayerState.h"

#include "AbilitySystemComponent.h"
#include "Character/Components/RDCCharacterAbilityComponent.h"

ARDCPlayerState::ARDCPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<URDCCharacterAbilityComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); 

	//same as character
	NetUpdateFrequency = 100.0f;
}

UAbilitySystemComponent* ARDCPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent; 
}

void ARDCPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
}


