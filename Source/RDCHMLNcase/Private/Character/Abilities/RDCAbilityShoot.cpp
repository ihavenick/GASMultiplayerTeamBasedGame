// Ata Çetin case


#include "Character/Abilities/RDCAbilityShoot.h"

#include "Character/RDCBaseCharacter.h"


URDCAbilityShoot::URDCAbilityShoot()
{
	AbilityInputID = EAbilityInputID::Shoot;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Shoot")));

	// We can set it in BP derived class but I want to use bp lowest I can
	AttackMontage = LoadObject<UAnimMontage>(AttackMontage,TEXT("/Game/Mannequin/Animations/AttackAnim.AttackAnim"));
}

void URDCAbilityShoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData * TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

		// Play montage if interface implemented
		if (const auto Character = ActorInfo->AvatarActor.Get(); Character->GetClass()->ImplementsInterface(URDCAbilitySystemInterface::StaticClass()))
			IRDCAbilitySystemInterface::Execute_PlayReplicatedMontage(Character, AttackMontage, 1.f, FName());
	}
}

bool URDCAbilityShoot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayTagContainer * SourceTags, const FGameplayTagContainer * TargetTags, OUT FGameplayTagContainer * OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	const ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);

	//if character is valid and not playing a montage then we can activate
	return Character && !Character->GetCurrentMontage();
}

void URDCAbilityShoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo != NULL && ActorInfo->AvatarActor != NULL)
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void URDCAbilityShoot::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &URDCAbilityShoot::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	//Stop montage if interface implemented and ability is cancelled
	if (const auto Character = ActorInfo->AvatarActor.Get(); Character->GetClass()->ImplementsInterface(URDCAbilitySystemInterface::StaticClass()))
		IRDCAbilitySystemInterface::Execute_StopReplicatedMontage(Character, AttackMontage);
}
