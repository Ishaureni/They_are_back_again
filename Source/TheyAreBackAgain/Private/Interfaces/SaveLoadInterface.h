// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveLoadInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class USaveLoadInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API ISaveLoadInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool OnSaveStarted_I();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnSaveFinished_I(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool OnLoadStarted_I(const FString& SlotName, const int32 UserIndex);	
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnLoadFinished_I(const bool bSuccess);
};
