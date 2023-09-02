// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "EAbilityID.generated.h"


UENUM(BlueprintType)
enum class EAbilityID : uint8
{
	None UMETA(DisplayName = "None"),
	Shoot UMETA(DisplayName = "Shoot"),
	Jump UMETA(DisplayName = "Jump"),
	SpawnProjectile UMETA(DisplayName = "SpawnProjectile")
};


UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None			UMETA(DisplayName = "None"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),
	Shoot			UMETA(DisplayName = "Shoot"),
	Jump			UMETA(DisplayName = "Jump"),
	SpawnProjectile UMETA(DisplayName = "SpawnProjectile")
};