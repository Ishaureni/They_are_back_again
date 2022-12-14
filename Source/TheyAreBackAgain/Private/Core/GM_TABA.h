// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/LevelManagerInterface.h"
#include "UserInterface/HUDManagerBase.h"
#include "Interfaces/SaveLoadInterface.h"
#include "GM_TABA.generated.h"

/**
 * 
 */
UCLASS()
class AGM_TABA : public AGameModeBase, public ISaveLoadInterface, public ILevelManagerInterface, public IUIInterface
{
	GENERATED_BODY()

public:
	/**HUD class that manages user interface*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AHUDManagerBase* HUDManager;

	AGM_TABA();
	
	virtual void BeginPlay() override;

	virtual bool OnSaveStarted_I_Implementation() override;

	virtual void OnSaveFinished_I_Implementation(const FString& SlotName, const int32 UserIndex, bool bSuccess) override;

	virtual bool OnLoadStarted_I_Implementation(const FString& SlotName, const int32 UserIndex) override;

	virtual void OnLoadFinished_I_Implementation(bool bSuccess) override;

	virtual void EnemyArrivedToTargetObject_Implementation(AActor* Enemy) override;
};
