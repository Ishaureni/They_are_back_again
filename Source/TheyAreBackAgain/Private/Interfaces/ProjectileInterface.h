// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectileInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UProjectileInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API IProjectileInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActivateProjectile_I(AActor* Target, double Damage);
	
};
