// Copyright Epic Games, Inc. All Rights Reserved.


#include "Game/RDCGameMode.h"

#include "Character/RDCBaseCharacter.h"
#include "Data/ETeam.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ARDCGameMode::ARDCGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Core/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ARDCGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AddPawnTeamInfo(NewPlayer); // Add Team Info to Pawn on login
}

void ARDCGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundActors);


	for (AActor* FoundActor : FoundActors) //get all spawn points and assign it to their arrays
	{
		if (APlayerStart* PlayerStart = Cast<APlayerStart>(FoundActor))
		{
			if (PlayerStart->PlayerStartTag == FName("TeamA"))
				TeamAStart.Push(PlayerStart);
			else if (PlayerStart->PlayerStartTag== FName("TeamB"))
				TeamBStart.Push(PlayerStart);
		}
			
	}

	UE_LOG(LogTemp, Warning, TEXT("A team has %d Spawnpoint , B team has %d spawnpoint"),TeamAStart.Num(),TeamBStart.Num());
}


AActor* ARDCGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (!Player->ActorHasTag(FName("HasTeam"))) // if player doesn't have team, assign it to a team
	{
		if (TeamA.Num() <= TeamB.Num())
		{
			TeamA.Add(Player->GetUniqueID());
			UE_LOG(LogTemp, Warning, TEXT("Adding player to Team A"));
		}
		else
		{
			TeamB.Add(Player->GetUniqueID());
			UE_LOG(LogTemp, Warning, TEXT("Adding player to Team B"));
		}
		Player->Tags.Add(FName("HasTeam"));
	}

	if(TeamAStart.Num() == 0 || TeamBStart.Num() == 0) // if there is no spawn point, return error
	{
		UE_LOG(LogTemp, Error, TEXT("TeamA or TeamB doesn't have spawn point"));
	}
	else
	{
		if (TeamA.Contains(Player->GetUniqueID())) // if player is in team A, assign it to a random A spawn point
		{
			const int32 Random = FMath::RandRange(0, TeamAStart.Num() - 1);
			if (TeamAStart[Random])
				return TeamAStart[Random];
			UE_LOG(LogTemp, Error, TEXT("TeamA doesn't have spawn point"));
		}
		if (TeamB.Contains(Player->GetUniqueID()))
		{
			const int32 Random = FMath::RandRange(0, TeamBStart.Num() - 1); // if player is in team B, assign it to a random B spawn point
			if (TeamBStart[Random])
				return TeamBStart[Random];
			UE_LOG(LogTemp, Error, TEXT("TeamB doesn't have spawn point"));
		}
		UE_LOG(LogTemp, Warning, TEXT("Could not find spawn point for PlayerID %d"), Player->GetUniqueID()); // if player is not in any team, return error
	}
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void ARDCGameMode::RestartPlayerwithTimer(AController* Controller)
{
	UE_LOG(LogTemp, Warning, TEXT("%d ID Player added to respawn list"),Controller->GetUniqueID()); // add player to respawn list
	RestartPlayer(Controller); 

	DeadPlayers.Add(Controller); // add player to dead player list
	GetWorldTimerManager().SetTimer(DeadTimer, this, &ARDCGameMode::Respawn, 4.f); // respawn player after 4 seconds (maybe resets other death timers) (maybe add a timer for each player)
}

void ARDCGameMode::AddPawnTeamInfo(AController* Controller) const
{
	const auto Pawn = Cast<ARDCBaseCharacter>(Controller->GetPawn());
	
	if (TeamA.Contains(Controller->GetUniqueID())) // add team info to pawn
	{
		Pawn->Team = ETeams::ETeam_A;
		Pawn->Tags.Add(FName("TeamA"));
		UE_LOG(LogTemp, Warning, TEXT("Assinged Pawn to Team A"));
	}
	else
	{
		Pawn->Team = ETeams::ETeam_B;
		Pawn->Tags.Add(FName("TeamB"));
		UE_LOG(LogTemp, Warning, TEXT("Assinged Pawn to Team B"));
	}
}

void ARDCGameMode::Respawn()
{
	auto CacheDeathPlayers = DeadPlayers; // cache dead players to avoid errors (can change while we iterating)
	for (auto DeadPlayer : CacheDeathPlayers)
	{
		AController* Controller = DeadPlayer;
	
		if (TeamA.Contains(Controller->GetUniqueID()))
		{
			UE_LOG(LogTemp, Warning, TEXT("%d ID Player going to spawn on A"),Controller->GetUniqueID());
			const int32 Random = FMath::RandRange(0,TeamAStart.Num()-1);
			RestartPlayerAtPlayerStart(Controller ,TeamAStart[Random]);
		}
		else if (TeamB.Contains(Controller->GetUniqueID()))
		{
			UE_LOG(LogTemp, Warning, TEXT("%d ID Player going to spawn on B"),Controller->GetUniqueID());
			const int32 Random = FMath::RandRange(0,TeamBStart.Num()-1);
			RestartPlayerAtPlayerStart(Controller ,TeamBStart[Random]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%d ID Player GG, Doesnt assigned to any teams"),Controller->GetUniqueID());
		}

		AddPawnTeamInfo(Controller);
		DeadPlayers.Remove(DeadPlayer);
	}
}
