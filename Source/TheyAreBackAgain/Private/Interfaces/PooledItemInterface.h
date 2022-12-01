// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PooledItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UPooledItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API IPooledItemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActivatePoolItem_I();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RegisterPool_I(UActorComponent* PoolManager);
	
};
