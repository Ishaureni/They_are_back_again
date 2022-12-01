// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API IUIInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OpenMainMenu();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CloseMainMenu();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ToggleHUD();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void QuitAndUnloadMatch();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ShowStatsWindow();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideStatsWindows();

	
};
