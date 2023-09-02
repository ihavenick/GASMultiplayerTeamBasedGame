// Ata Çetin case


#include "Character/Components/RDCHealthComponent.h"

#include "Character/RDCBaseCharacter.h"
#include "Game/RDCGameMode.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	DefaultHealth = 100; // Default health (getting this from save or something would be better)
	bIsDead = false; // Not dead on default
	
	SetIsReplicatedByDefault(true); // This component is replicated by default
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();


	if (GetOwnerRole() == ROLE_Authority)
		if (AActor* MyOwner = GetOwner())
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage); // Bind to OnTakeAnyDamage event of owner if in server

	Health = DefaultHealth; // Set health to default
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	const float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr); // Broadcast health changed event
}


void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy,
	AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead) // If damage is less than or equal to 0 or dead, return (Why you want to kill already death :D)
		return;
	
	const ARDCBaseCharacter* PawnDamaged = Cast<ARDCBaseCharacter>(DamagedActor);
	const ARDCBaseCharacter* PawnDamager = Cast<ARDCBaseCharacter>(DamageCauser);

	if (!PawnDamaged || !PawnDamager)
		return;

	//Checks if the damage causer is the same as the damaged actor or if they are in the same team
	//Implementing Team getting interface will be better maybe
	if (DamageCauser == DamagedActor || PawnDamaged->Team == PawnDamager->Team)  
		return;

	// Update health clamped
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	bIsDead = Health <= 0.0f; // If health is less than or equal to 0, set bIsDead to true

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser); // Broadcast health changed event

	
	if (ARDCGameMode* Gm = Cast<ARDCGameMode>(GetWorld()->GetAuthGameMode()); bIsDead && Gm) // If dead and game mode is valid
		Gm->RestartPlayerwithTimer(PawnDamaged->GetController()); // Restart player with timer
}


float UHealthComponent::GetHealth() const
{
	return Health; // Return health
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health); // Replicate health
}

