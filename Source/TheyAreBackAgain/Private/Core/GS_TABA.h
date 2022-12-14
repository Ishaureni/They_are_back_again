// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GS_TABA.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyDied, bool, bIsFlying, FTransform, DiedAt);

/**
 * 
 */
UCLASS(BlueprintType)
class AGS_TABA : public AGameStateBase
{
	GENERATED_BODY()

public:

	/**Int representing the destroyed enemies*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlyingKilled = 0;

	/**Int representing the destroyed enemies*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GroundKilled = 0;

	/**Int representing the destroyed enemies*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemiesLeftInWave = 0;

	/**Int representing the destroyed enemies*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemiesLeftTotal = 0;
	
	/**Int representing the destroyed enemies*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Victory = false;

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void OnEnemyDied(bool bIsFlyingEnemy, FTransform DiedAt);

	UPROPERTY(BlueprintAssignable)
	FOnEnemyDied OnEnemyDiedEvent;

};
