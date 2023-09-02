// Ata Çetin case


#include "Character/Abilities/RDCAbilitySpawnProjectile.h"

#include "GameFramework/Character.h"
#include "Interfaces/RDCAbilitySystemInterface.h"
#include "World/Actors/RDCProjectileActor.h"

URDCAbilitySpawnProjectile::URDCAbilitySpawnProjectile()
{
	AbilityInputID = EAbilityInputID::SpawnProjectile;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.SpawnProjectile")));
}

void URDCAbilitySpawnProjectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData * TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}

		APawn* Character = CastChecked<APawn>(ActorInfo->AvatarActor.Get());
		//Checking if actor has authority ( there was a double spawn bug but it coused because i put double notify, so i dont know if this is needed)
		if (ActorInfo->AvatarActor.Get()->HasAuthority()) 
		{
			//geting world of character is better
			if (auto World = Character->GetWorld(); World != nullptr && Character != nullptr)
			{
				//Checking if character implements interface
				if (Character->GetClass()->ImplementsInterface(URDCAbilitySystemInterface::StaticClass()))
				{
					FRotator SpawnRotation;
					FVector SpawnLocation;
					TSubclassOf<AProjectileActor> ProjectileClass;
					IRDCAbilitySystemInterface::Execute_GetProjectileVariables(Character, SpawnRotation, SpawnLocation,ProjectileClass);  // Getting spawn variables from character

					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					//Spawning projectile
					if (const auto Projectile = World->SpawnActor<AProjectileActor>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams))
					{
						Projectile->SetDamagerOwner(Character); //seting damager to log kiils in future
						// Log projectile location
						UE_LOG(LogTemp, Warning, TEXT("Projectile Spawn Loc: %s"), *Projectile->GetActorLocation().ToString());
					}
					else  // Houston we have a problem, Debug time
					{
						UE_LOG(LogTemp, Warning, TEXT("SpawnProjectile failed to spawn ProjectileClass"));
						EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // Ending ability if spawn failed
						//Debug Spawn Location and ProjectileClass
						UE_LOG(LogTemp, Warning, TEXT("SpawnLocation: %s"), *SpawnLocation.ToString());
						if (ProjectileClass)
						{
							UE_LOG(LogTemp, Warning, TEXT("ProjectileClass: %s"), *ProjectileClass->GetName());
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("ProjectileClass is null"));
						}
					}
				}
				else // Character does not implement interface
				{
					UE_LOG(LogTemp, Warning, TEXT("Character does not implement URDCAbilitySystemInterface"));
					EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				}
			}
			else // World or Character is null
			{
				UE_LOG(LogTemp, Warning, TEXT("World or Character is null"));
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			}
		}
	}
}

bool URDCAbilitySpawnProjectile::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayTagContainer * SourceTags, const FGameplayTagContainer * TargetTags, OUT FGameplayTagContainer * OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	const ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	return IsValid(Character); // Maybe we can write ammo count check in future
}

void URDCAbilitySpawnProjectile::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo != NULL && ActorInfo->AvatarActor != NULL)
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void URDCAbilitySpawnProjectile::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &URDCAbilitySpawnProjectile::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	
}