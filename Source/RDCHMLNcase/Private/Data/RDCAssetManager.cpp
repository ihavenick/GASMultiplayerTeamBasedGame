// Ata Çetin case


#include "Data/RDCAssetManager.h"

#include "AbilitySystemGlobals.h"

void URDCAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	UAbilitySystemGlobals::Get().InitGlobalData();
	
	UE_LOG(LogTemp, Warning, TEXT("Asset manager started loading"));
}
