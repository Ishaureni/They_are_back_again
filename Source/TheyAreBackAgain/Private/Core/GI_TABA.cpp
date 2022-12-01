// Copyright Ónodi Kristóf. All Rights Reserved.


#include "Core/GI_TABA.h"
#include "Commandlets/WorldPartitionCommandletHelpers.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"


double UGI_TABA::GetGlobalScalingFactor() const
{
	return GlobalScalingFactor;
}

double UGI_TABA::GetGlobalGridSize() const
{
	return GlobalGridSize;
}

double UGI_TABA::GetGlobalGridPadding() const
{
	return GlobalGridPadding;
}

void UGI_TABA::SetGlobalScalingFactor(const double Value)
{
	GlobalScalingFactor = Value;
}

void UGI_TABA::SetGlobalGridSize(const double Value)
{
	GlobalGridSize = Value;
}

void UGI_TABA::SetGlobalGridPadding(const double Value)
{
	GlobalGridPadding = Value;
}

FName UGI_TABA::GetPlayerName() const
{
	return CurrentPlayerName;
}

void UGI_TABA::SetPlayerName(FName NewName)
{
	CurrentPlayerName = NewName;
}

int32 UGI_TABA::GetCurrentLevelID() const
{
	return CurrentLevelID;
}

void UGI_TABA::SetCurrentLevelID(int32 NewLevelID)
{
	CurrentLevelID = NewLevelID;
}

#pragma region Save-Load

#pragma region Save
void UGI_TABA::TrySaveGame(const FString SaveSlot)
{
	if(!Cast<ISaveLoadInterface>(GameMode))
	{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: Failed to message save game interface on HUD"), *this->GetName());
			return;
	}
	
	if (USaveGameBase* LocalSaveGame = Cast<USaveGameBase>(UGameplayStatics::CreateSaveGameObject(USaveGameBase::StaticClass())))
	{
		if(!Cast<ISaveLoadInterface>(GameMode))Execute_OnSaveStarted_I(GameMode);
		
		// Assign OnSaved delegate
		FAsyncSaveGameToSlotDelegate OnSaved;
		OnSaved.BindUObject(this, &UGI_TABA::OnSaveFinished);
		
		//Set saved variables 
		LocalSaveGame->PlayerName = CurrentPlayerName.ToString();

		//TODO Set remaining save data
		
		//Save async
		UGameplayStatics::AsyncSaveGameToSlot(LocalSaveGame, SaveSlot, 0, OnSaved);
	}
}

void UGI_TABA::OnSaveFinished(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	Execute_OnSaveFinished_I(GameMode, SlotName, UserIndex, bSuccess);
}
#pragma endregion Save

#pragma region Load

void UGI_TABA::TryLoadGame(const FString SaveSlot)
{
	if(!Cast<ISaveLoadInterface>(GameMode))
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: Failed to message save game interface on HUD"), *this->GetName());
		return;
	}
	
	Execute_OnLoadStarted_I(GameMode, SaveSlot, 0);
	
	// OnLoaded delegate
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &UGI_TABA::OnLoaded);

	//Load async
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlot, 0, LoadedDelegate);
}

void UGI_TABA::OnLoaded(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	//Check if load data valid
	if(USaveGameBase* LocalLoadedData = Cast<USaveGameBase>(LoadedGameData))
	{
		// Load game data
		//TODO Implement load data
		SetPlayerName(FName(LocalLoadedData->PlayerName));
		
		Execute_OnLoadFinished_I(GameMode, true);
		return;
	}


	// Load failed for some reason, loaded data is null
	Execute_OnLoadFinished_I(GameMode, false);
	
}
#pragma endregion Load

#pragma endregion Save-Load




