// Ata Çetin case


#include "Character/Abilities/RDCGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

URDCGameplayAbilityBase::URDCGameplayAbilityBase()
{
	// Default to Instance Per Actor
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // This ability is instanced per actor (on default)

	// Default tags that block this ability from activating
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dead"))); // If dead doesn't activate
}

void URDCGameplayAbilityBase::OnAvatarSet(const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilitySpec & Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
}