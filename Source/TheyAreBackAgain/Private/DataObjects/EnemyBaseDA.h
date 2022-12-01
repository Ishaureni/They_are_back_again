// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyBaseDA.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class THEYAREBACKAGAIN_API UEnemyBaseDA : public UDataAsset
{
	GENERATED_BODY()
		
public:

	/** Display name */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName EnemyName = "NewEnemy";

	/** Set to true if the enemy can fly
	 *  If false only ground lane can spawn them
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool IsFlying = true;

	/** Initial health pool */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double MaxHealth = 100.0;

	/** Base speed without modifiers */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double Speed = 5.0;

	/** Base mesh */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> MeshPtr;

	/** Should MI color be overridden */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool OverrideColor = false;

	/** If bOverrideColor = true, this color will be used. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FLinearColor ColorOverride;
	
};
