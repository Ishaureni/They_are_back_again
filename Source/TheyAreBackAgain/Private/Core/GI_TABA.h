// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

#include "Interfaces/SaveLoadInterface.h"

#include "GI_TABA.generated.h"

/**
 * 
 */
UCLASS()
class UGI_TABA : public UGameInstance, public ISaveLoadInterface
{
	GENERATED_BODY()

private:

	/**Currently used save game slot*/
	UPROPERTY(VisibleAnywhere, Category="State|SaveData")
	FString CurrentSaveGame = "NewSaveSlot";
	
	/**Name of the current local player*/
	UPROPERTY(BlueprintSetter=SetPlayerName, BlueprintGetter=GetPlayerName, VisibleAnywhere, Category="State")
	FName CurrentPlayerName = "New Player";

	/**Current level ID*/
	UPROPERTY(VisibleAnywhere, BlueprintGetter=GetCurrentLevelID, BlueprintSetter=SetCurrentLevelID, Category="State")
	int32 CurrentLevelID = 0;

	/**The size of a default 1x1 tile in cm*/
	UPROPERTY(BlueprintGetter=GetGlobalGridSize, BlueprintSetter=SetGlobalGridSize, VisibleAnywhere, Category="Core settings")
	double GlobalGridSize = 100.0;

	/**Default padding between the tiles generated in cm*/
	UPROPERTY(BlueprintGetter=GetGlobalGridPadding, BlueprintSetter=SetGlobalGridPadding, VisibleAnywhere, Category="Core settings")
	double GlobalGridPadding = 1.0;

	/**Overall map scaling factor*/
	UPROPERTY(BlueprintGetter=GetGlobalScalingFactor, BlueprintSetter=SetGlobalScalingFactor, VisibleAnywhere, Category="Core settings")
	double GlobalScalingFactor = 2;
	
	/**GameMode that manages game core functions*/
	UPROPERTY(Transient)
	AActor* GameMode;

public:
	FStreamableManager StreamableManager = FStreamableManager();
	
	/**Getter for ScalingFactor*/
	UFUNCTION(BlueprintGetter)
	double GetGlobalScalingFactor() const;

	/**Setter for ScalingFactor*/
	UFUNCTION(BlueprintSetter)
	void SetGlobalScalingFactor(const double Value);
				
	/**Getter for grid size*/
	UFUNCTION(BlueprintGetter)
	double GetGlobalGridSize() const;

	/**Setter for grid size*/
	UFUNCTION(BlueprintSetter)
	void SetGlobalGridSize(const double Value);
		
	/**Getter for grid padding*/
	UFUNCTION(BlueprintGetter)
	double GetGlobalGridPadding() const;

	/**Setter for grid padding*/
	UFUNCTION(BlueprintSetter)
	void SetGlobalGridPadding(const double Value);
	
	/**Getter for current player name*/
	UFUNCTION(BlueprintGetter)
	FName GetPlayerName() const;

	/**Setter current player name*/
	UFUNCTION(BlueprintSetter)
	void SetPlayerName(FName NewName);

	/**Getter for current level asset*/
	UFUNCTION(BlueprintGetter)
	int32 GetCurrentLevelID() const;

	/**Setter for current level asset*/
	UFUNCTION(BlueprintSetter)
	void SetCurrentLevelID(int32 NewLevelID);


#pragma region Save-Load
	// Save and Load functions //
	UFUNCTION(BlueprintCallable)
	void TrySaveGame(FString SaveSlot);

	// Save and Load functions //
	UFUNCTION(BlueprintCallable)
	void OnSaveFinished(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	UFUNCTION(BlueprintCallable)
	void TryLoadGame(FString SaveSlot);
	
	/**Delegate f for saved loading finished*/
	void OnLoaded(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData);

#pragma endregion Save-Load

	
};

