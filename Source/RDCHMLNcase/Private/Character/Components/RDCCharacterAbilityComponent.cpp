﻿// Ata Çetin case


#include "Character/Components/RDCCharacterAbilityComponent.h"


// Sets default values for this component's properties
URDCCharacterAbilityComponent::URDCCharacterAbilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URDCCharacterAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void URDCCharacterAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


// Untouched