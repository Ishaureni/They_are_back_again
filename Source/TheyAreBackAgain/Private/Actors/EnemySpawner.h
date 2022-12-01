// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LevelMaster.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SpawnerInterface.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"
#include "EnemySpawner.generated.h"

UCLASS()
class THEYAREBACKAGAIN_API AEnemySpawner : public AActor, public ISpawnerInterface, public IPooledItemInterface
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<AEnemyBase>> DefaultEnemies;

	UPROPERTY(VisibleAnywhere)
	TArray<FGrid2D> Waypoints;

	UPROPERTY(VisibleAnywhere)
	TArray<FGrid2D> GroundWaypoints;

	/**Air units spawn position placeholder
	 * Default is height offset
	 */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* AirSpawnPosition;

	/**Ground units spawn position placeholder
	 * Default is height offset
	 */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* GroundSpawnPosition;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsGroundLane;
	
	UPROPERTY(VisibleAnywhere)
	int32 LaneID;

	UPROPERTY(VisibleAnywhere)
	bool IsActive = false;
	
	UPROPERTY(Transient, VisibleAnywhere)
	int32 CurrentWaveID;

	UPROPERTY(Transient, VisibleAnywhere)
	TArray<int32> CurrentWaveSpawnLeft;

	UPROPERTY(Transient, VisibleAnywhere)
	int32 TotalSpawnedAmount = 0;
	
	UPROPERTY(VisibleAnywhere)
	UObjectPoolManagerComponent* ObjectPoolManager;

	UPROPERTY(VisibleAnywhere)
	ALevelMaster* LevelMaster;
	
	// - Currently loaded lane-global air enemy asset
	// Set on Init
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	TArray<UEnemyBaseDA*> LoadedAirEnemies;

	// - Currently loaded lane-global ground enemy asset
	// Set on Init
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	TArray<UEnemyBaseDA*> LoadedGroundEnemies;
	
	// - Currently loaded wave air enemy asset
	// Set on wave start
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	TArray<UEnemyBaseDA*> LoadedWaveAirEnemies;

	// - Currently loaded wave ground enemy asset
	// Set on wave start
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	TArray<UEnemyBaseDA*> LoadedWaveGroundEnemies;

	UPROPERTY()
	FTimerHandle SpawnTimerHandle;
	
protected:
	virtual void BeginPlay() override;

public:

	/**Activates spawner
	 * Called at first wave
	 */
	void ActivateSpawner();

	/**Deactivates spawner
	 * Fires when spawner has no more waves.
	 */
	UFUNCTION()
	void DeActivateSpawner(bool bLevelDefeated = false);

	/**Called when all spawners finished wave*/
	UFUNCTION()
	void StartNextWave(const int32 WaveIndex);

	/**Setup spawner*/
	virtual void InitSpawnerActor_I_Implementation(FTransform& Transform, const int32 LaneIndex, const FGrid2D TargetCoord, const FEnemyLane& LaneInfo) override;

	/**Can be used by pool manager for additional task on creation*/
	virtual void ActivatePoolItem_I_Implementation() override;

	/**Called by object pool upon creation*/
	virtual void RegisterPool_I_Implementation(UActorComponent* PoolManager) override;

	/**If waypoints are not linear tries to insert points for a valid road generation.*/
	bool TryCalculateStraightenerCoord(FGrid2D& OutCoord, const FGrid2D From, const FGrid2D To, const int32 LaneIndex, const TArray<FGrid2D>& OriginalWaypoints) const;

	/**Checks if given waypoints can be used*/
	void ValidateWaypoints(const FGrid2D TargetCoord, const FEnemyLane& LaneInfo, const int32 LaneIndex);

	/**Tries to render all roads*/
	void SpawnRoad(const FGrid2D TargetCoord, const int32 LaneIndex);

	/**Getter for loaded assets
	 * @param EnemyArray array ref to select a random from
	 */
	UFUNCTION(BlueprintCallable)
	UEnemyBaseDA* GetRandomEnemy(const TArray<UEnemyBaseDA*>& EnemyArray) const;

	/**Spawns a random enemy from the different pools*/
	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();

	/**Called when poolmanager OnPoolWiped delegate is broadcasted*/
	UFUNCTION()
	void DestroySelf();
};
