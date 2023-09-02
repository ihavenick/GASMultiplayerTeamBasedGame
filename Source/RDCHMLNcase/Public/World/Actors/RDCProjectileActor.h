// Ata Çetin Case

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RDCProjectileActor.generated.h"

class USoundCue;
class UProjectileMovementComponent;
class USphereComponent;
UCLASS()
class AProjectileActor : public AActor
{

	UPROPERTY(Replicated)
	APawn* DamageCauser;
	
	GENERATED_BODY()
	UPROPERTY(VisibleDefaultsOnly)
	USphereComponent* CollisionComp;
	UPROPERTY(VisibleDefaultsOnly, Category = "RDC | Efects")
	UParticleSystemComponent* Fireball;

	UPROPERTY(EditDefaultsOnly, Category = "RDC | Efects")
	USoundCue* ImpactSFX;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bDebugEnabled = false;
	
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Sets default values for this actor's properties
	AProjectileActor();
	
	UFUNCTION()
	void SetDamagerOwner(APawn* Pawn);
	UFUNCTION()
	APawn* GetDamagerOwner();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RDC | Efects")
	UParticleSystem* FireParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RDC | Efects")
	UParticleSystem* ImpactVFX;
	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

};
