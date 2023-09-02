// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "ETeam.generated.h"


UENUM(BlueprintType)
enum class ETeams : uint8
{
 
	ETeam_A            UMETA(DisplayName = "A Team"),
	ETeam_B            UMETA(DisplayName = "B Team"),
 
};
