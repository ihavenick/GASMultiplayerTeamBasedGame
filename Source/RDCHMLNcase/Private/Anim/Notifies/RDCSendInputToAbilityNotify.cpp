// Ata Çetin case


#include "Anim/Notifies/RDCSendInputToAbilityNotify.h"

#include "Interfaces/RDCAbilitySystemInterface.h"

void URDCSendInputToAbilityNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	//Check if the mesh component is valid
	if(MeshComp)
		//Get and check if the owner is valid
		if(const auto Actor = MeshComp->GetOwner())
			//Check if the owner implements the interface
			if (Actor->GetClass()->ImplementsInterface(URDCAbilitySystemInterface::StaticClass()))
				//Send the input to the ability system
				IRDCAbilitySystemInterface::Execute_SendInputtoAbility(Actor, AbilityInputID, bPressed);
}
