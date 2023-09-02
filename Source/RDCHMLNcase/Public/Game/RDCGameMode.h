// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RDCGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);

UCLASS(minimalapi)
class ARDCGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	TArray<uint32> TeamA;
	TArray<uint32> TeamB;

	TArray<class APlayerStart*> TeamAStart;
	TArray<APlayerStart*> TeamBStart;

	UPROPERTY()
	TArray<AController*> DeadPlayers;
	FTimerHandle DeadTimer;

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

public:
	ARDCGameMode();

	UFUNCTION(BlueprintCallable)
	void Respawn();
	
	UFUNCTION()
	void RestartPlayerwithTimer(AController* Controller);
	
	void AddPawnTeamInfo(AController* Controller) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActorKilled OnActorKilled;
};



