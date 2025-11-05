// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionSubsystem.h"

void PrintString(const FString & String)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,String);
}

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem()
{
    //PrintString("Subsystem Constructor");
    CreateServerAfterDestroy=false;
    DestroyServerName="";
    ServerNameToFind="";
    MySessionName="MultiplayerSubsystem";
}

void UMultiplayerSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    //PrintString("UMultiplayerSessionSubsystem::Initialize");

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        FString SubsystemName= OnlineSubsystem->GetSubsystemName().ToString();
        PrintString(SubsystemName);

        SessionInterface= OnlineSubsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,&UMultiplayerSessionSubsystem::OnCreateSessionComplete);

            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,&UMultiplayerSessionSubsystem::OnDestroySessionComplete);

            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,&UMultiplayerSessionSubsystem::OnFindSessionComplete);

            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,&UMultiplayerSessionSubsystem::OnJoinSessionComplete);
        }
    }
}

void UMultiplayerSessionSubsystem::Deinitialize()
{
    //UE_LOG(LogTemp, Warning,TEXT("UMultiplayerSessionSubsystem::Deinitialize") );
}

void UMultiplayerSessionSubsystem::CreateServer(FString ServerName)
{
    PrintString(ServerName);

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bIsDedicated = false;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUseLobbiesIfAvailable = true;
    SessionSettings.NumPublicConnections=2;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bAllowJoinViaPresence = true;
    if(IOnlineSubsystem::Get()->GetSubsystemName()=="NULL")
        SessionSettings.bIsLANMatch=true;
    else
    {
        SessionSettings.bIsLANMatch=false;
    }

    FNamedOnlineSession* ExistingSession= SessionInterface->GetNamedSession(MySessionName);
    if (ExistingSession)
    {
        CreateServerAfterDestroy=true;
        DestroyServerName=ServerName;
        SessionInterface->DestroySession(MySessionName);
        return;
    }

    SessionSettings.Set(FName("SERVER_NAME"),ServerName,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->CreateSession(0,MySessionName, SessionSettings);
}

void UMultiplayerSessionSubsystem::FindServer(FString ServerName)
{
    PrintString(ServerName);

    SessionSearch= MakeShareable(new FOnlineSessionSearch());
    if(IOnlineSubsystem::Get()->GetSubsystemName()=="NULL")
        SessionSearch->bIsLanQuery=true;
    else
    {
        SessionSearch->bIsLanQuery=false;
    }
    SessionSearch->MaxSearchResults=100;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE,true, EOnlineComparisonOp::Equals);

    ServerNameToFind=ServerName;

    SessionInterface->FindSessions(0,SessionSearch.ToSharedRef());
}

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    PrintString(FString::Printf(TEXT("OnCreateSessionComplete: %d"), bWasSuccessful));

    if(bWasSuccessful)
    {
        FString DefaultGameMapPath="/Game/ThirdPerson/Maps/ThirdPersonMap?listen";

        if(!GameMapPath.IsEmpty())
        {
            GetWorld()->ServerTravel(GameMapPath+"?listen");
        }
        else
        {
            GetWorld()->ServerTravel(DefaultGameMapPath);
        }
    }
}

void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if(CreateServerAfterDestroy)
    {
        CreateServerAfterDestroy=false;
        CreateServer(DestroyServerName);
    }
}

void UMultiplayerSessionSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
    if(!bWasSuccessful)
        return;

    if(ServerNameToFind.IsEmpty())
        return;

    TArray<FOnlineSessionSearchResult> Results=SessionSearch->SearchResults;
    FOnlineSessionSearchResult* CorrectResult= 0;

    if(Results.Num()>0)
    {
        for(FOnlineSessionSearchResult Result:Results)
        {
            if(Result.IsValid())
            {
                FString ServerName="No-Name";
                Result.Session.SessionSettings.Get(FName("SERVER_NAME"),ServerName);

                if(ServerName.Equals(ServerNameToFind))
                {
                    CorrectResult=&Result;
                    break;
                }
            }
        }
        if(CorrectResult)
        {
            SessionInterface->JoinSession(0,MySessionName,*CorrectResult);
        }
    }
    else
    {
        PrintString("OnFindSessionComplete: No sessions found");
    }
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if(Result==EOnJoinSessionCompleteResult::Success)
    {
        PrintString("OnJoinSessionComplete: Success");
        FString Address= "";

        bool Success= SessionInterface->GetResolvedConnectString(MySessionName,Address);
        if(Success)
        {
            PrintString(FString::Printf(TEXT("Address: %s"), *Address));
            APlayerController* PlayerController= GetGameInstance()->GetFirstLocalPlayerController();

            if(PlayerController)
            {
                PrintString("ClientTravelCalled");
                PlayerController->ClientTravel(Address,TRAVEL_Absolute);
            }
        }
        else
        {
            PrintString("OnJoinSessionComplete: Failed");
        }
    }
}

void UMultiplayerSessionSubsystem::TravelToNewLevel(FString NewLevelPath)
{
    //Travel to new level with the connected client

    GetWorld()->ServerTravel(NewLevelPath+"?listen",true);
}