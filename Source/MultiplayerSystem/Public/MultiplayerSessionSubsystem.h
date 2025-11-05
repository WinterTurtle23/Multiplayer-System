// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/EngineTypes.h"

#include "MultiplayerSessionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSYSTEM_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BlueprintCallable, Category="Multiplayer System")
	void CreateServer(FString ServerName);

	UFUNCTION(BlueprintCallable, Category="Multiplayer System")
	void FindServer(FString ServerName);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	bool CreateServerAfterDestroy;
	FString DestroyServerName;
	FString ServerNameToFind;

	FName MySessionName;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	void OnFindSessionComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UPROPERTY(BlueprintReadWrite, Category="Multiplayer System")
	FString GameMapPath;

	UFUNCTION(BlueprintCallable, Category="Multiplayer System")
	void TravelToNewLevel(FString NewLevelPath);
};
