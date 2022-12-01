// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AudioInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UAudioInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API IAudioInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	void PlayShoot();
	
};
