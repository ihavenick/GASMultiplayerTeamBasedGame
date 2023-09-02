// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RDCCharacterAbilityComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RDCHMLNCASE_API URDCCharacterAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	//Varibles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RDC | Variables")
	bool bIsAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;


	// Sets default values for this component's properties
	URDCCharacterAbilityComponent();

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
