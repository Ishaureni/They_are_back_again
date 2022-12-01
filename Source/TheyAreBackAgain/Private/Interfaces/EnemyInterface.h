// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataObjects/EnemyBaseDA.h"
#include "DataObjects/Structures.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEYAREBACKAGAIN_API IEnemyInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void InitEnemy_I(const UEnemyBaseDA* EnemyData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActivateEnemy_I(const TArray<FGrid2D>& Waypoints);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TakeDamage_I(const double Damage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead();
};
