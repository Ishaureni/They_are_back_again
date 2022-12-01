// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameBase.generated.h"

/**
 * 
 */
UCLASS()
class USaveGameBase final : public USaveGame
{
	GENERATED_BODY()


public:
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName = "NewSaveGame";

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex = 0;

	/**Player name save data*/
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString PlayerName;

	/**Next level to load*/
	UPROPERTY(VisibleAnywhere, Category=State)
	int16 NextLevel = 0;

	/**Total time played (excluding menu)*/
	UPROPERTY(VisibleAnywhere, Category=Misc)
	double PlayedTime = 0.0;
};
