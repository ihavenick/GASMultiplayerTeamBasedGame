// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "RDCAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class RDCHMLNCASE_API URDCAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	virtual void StartInitialLoading() override;
};
