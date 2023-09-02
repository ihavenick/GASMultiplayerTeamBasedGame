// Ata Çetin case

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "RDCHealthBarWidget.generated.h"


class ARDCBaseCharacter;
class UProgressBar;
/**
 * 
 */
UCLASS()
class UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	bool Green;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY()
	ARDCBaseCharacter* Owner = nullptr;

	
	virtual void NativeConstruct() override;
	
public:
	UProgressBar* GetHealthBar() const;
	void SetOwner(ARDCBaseCharacter* NewOwner) { Owner = NewOwner; }

	UFUNCTION()
	void SetColorGreen() const;

	UFUNCTION(BlueprintPure, Category = "Progress Bar")
	FLinearColor GetProgressBarColor();
};
