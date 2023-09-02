// Ata Çetin case


#include "World/Actors/RDCProjectileActor.h"

#include "DrawDebugHelpers.h"
#include "Character/RDCBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"




// Sets default values
AProjectileActor::AProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileActor::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	
	// Set as root component
	RootComponent = CollisionComp;
	
	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 5.0f;

	bReplicates = true;
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectileActor, DamageCauser);
}

void AProjectileActor::SetDamagerOwner(APawn* Pawn)
{
	SetInstigator(Pawn); //owner of projectile
}

APawn* AProjectileActor::GetDamagerOwner()
{
	return GetInstigator(); //We couldnt get intigator soo this is getter
}

void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	Fireball = UGameplayStatics::SpawnEmitterAttached(FireParticle,CollisionComp);
}

void AProjectileActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!GetDamagerOwner())
	{
		return;
	}
	
	if (HasAuthority()) // damage only on server
	{
		TArray<AActor*> IgnoredActors;

		if (ARDCBaseCharacter* Pawn = Cast<ARDCBaseCharacter>(OtherActor))
			Pawn->TakeDamage(20.f,FDamageEvent(),nullptr,GetDamagerOwner());
		
		
		SetLifeSpan(2.f);
	}
	else
	{  // Have vfx but i coudnt use one (only activate on client)
		if(Fireball) //trail
			Fireball->SetVisibility(false);
		if (ImpactSFX) // impact sount
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSFX, GetActorLocation());
		if(ImpactVFX) //impact vfx
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactVFX,Hit.ImpactPoint);

		if(bDebugEnabled)
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 50, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	}
}

