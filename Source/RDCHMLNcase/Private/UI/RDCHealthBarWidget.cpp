// Ata Çetin case


#include "UI/RDCHealthBarWidget.h"


#include "Character/RDCBaseCharacter.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"


void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HealthBar->FillColorAndOpacityDelegate.BindUFunction(this, FName("GetProgressBarColor")); //bind color getter per team
}

UProgressBar* UHealthBarWidget::GetHealthBar() const
{
	return HealthBar; // gets healthbar
}

void UHealthBarWidget::SetColorGreen() const
{
	if (HealthBar)
		HealthBar->SetFillColorAndOpacity(FLinearColor::Green); //Deprecated color setter
}


FLinearColor UHealthBarWidget::GetProgressBarColor() // Gets teams and sets color of progressbar 
{
	if(const auto PlayerPawn = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)) //Compare owner with local player
		if(const auto PlayerChar = Cast<ARDCBaseCharacter>(PlayerPawn); Owner && PlayerChar)
			return Owner->Team == PlayerChar->Team ? FLinearColor::Green : FLinearColor::Red;

	return FLinearColor::Red;
	
}