// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Interfaces/UIInterface.h"
#include "HUDManagerBase.generated.h"



UCLASS()
class AHUDManagerBase : public AHUD, public IUIInterface
{
	GENERATED_BODY()

public:

	/**Current Game instance*/
	UPROPERTY(VisibleAnywhere)
	UGameInstance* GameInstance;

	virtual void BeginPlay() override;
	// Save - Load events
	
	void OnSaveStarted();
	void OnSavingFinished(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	void OnLoadStarted(const FString& SlotName, const int32 UserIndex);
	void OnLoaded(bool bSuccess);

	

};
