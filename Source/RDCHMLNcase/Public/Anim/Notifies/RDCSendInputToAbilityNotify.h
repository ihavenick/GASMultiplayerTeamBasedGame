// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Data/EAbilityID.h"
#include "RDCSendInputToAbilityNotify.generated.h"


/**
 * 
 */
UCLASS()
class RDCHMLNCASE_API URDCSendInputToAbilityNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RDC | Ability")
	EAbilityInputID AbilityInputID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RDC | Ability")
	bool bPressed;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
