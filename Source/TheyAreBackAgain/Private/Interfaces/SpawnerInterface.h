// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"
#include "SpawnerInterface.generated.h"

UINTERFACE(Blueprintable)
class USpawnerInterface : public UInterface
{
	GENERATED_BODY()
};



class THEYAREBACKAGAIN_API ISpawnerInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void InitSpawnerActor_I(FTransform& Transform, const int32 LaneIndex, const FGrid2D TargetCoord, const FEnemyLane& LaneInfo);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RequestRoad_I(const FGrid2D AtGrid, const ETileType RoadType, const EForwardDirection ForwardDirection, const FName Requestor = FName(""));

	/*
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RequestEnemy(const FName Enemy, FTransform SpawnAt, AActor* Spawner);
	*/

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool RequestRandomBool_I(const double Weight = 0.5);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int32 RequestRandomRangeInt_I(const int32 Max, const int32 Min = 0);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	double RequestRandomFloatRange_I(const FDoubleRange Range);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnemyDied_I();
};
