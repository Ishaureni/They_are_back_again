// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LevelManagerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class ULevelManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API ILevelManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FName> GetAvailableLevelDataAssets();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TryLoadLevelWithIndex(int32 PrimaryAssetIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnemyArrivedToTargetObject(AActor* Enemy);
	
};
