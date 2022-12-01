// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enums.h"
#include "DataObjects/EnemyBaseDA.h"
#include "Structures.generated.h"

USTRUCT(BlueprintType)
struct FGrid2D
{
	GENERATED_USTRUCT_BODY()



public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y;
	
	FGrid2D()
	{
		X=Y=0;
	};

	FGrid2D(const int32 NewX, const int32 NewY)
	{
		X=NewX;
		Y=NewY;
	};

	bool IsContained(const TArray<FGrid2D>& InCoordsArray) const;

	bool Equals(const FGrid2D& OtherCoord) const;

	bool IsVerticallyAligned(const FGrid2D& OtherCoord) const;
	
	bool IsHorizontallyAligned(const FGrid2D& OtherCoord) const;

	bool IsStraight(const FGrid2D& OtherCoord) const;

	bool IsBetween(const FGrid2D& A, const FGrid2D& B) const;

	/**Calculates the steps required to B, allowing only linear movement*/
	int32 LinearDistance(const FGrid2D& B) const;

	/**Adds 'FIntCoord& B' to the calling coord
	 * @param Scale multiples B before addition
	 */ 
	FGrid2D AddScaled(const FGrid2D& B, const int32 Scale = 1) const;
	
	//Subtracts 'FIntCoord& B' from the calling coord 
	FGrid2D Subtract(const FGrid2D& B) const;

	//Returns linear directions in each axis
	FGrid2D Unit() const;

	//Returns forward direction enum
	EForwardDirection GetForwardDirection() const;
	
	FGrid2D Clamp(FGrid2D Min, FGrid2D Max) const;

	static FGrid2D Zero();

	FString ToString() const;

	/**Mixes up coordinates
	 * @param bSwap set true for swap returned members
	 * @return FGrid( X, OtherCoord.Y) by default
	 * @return if bSwap = true, FGrid( OtherCoord.X, Y)
	 */
	FGrid2D Mix(const FGrid2D& OtherCoord, const bool bSwap = false) const;
	
	
};


/**Defines:
 * - enemies to spawn
 * - enemy amount in wave
 * - spawning timers of wave
 */
USTRUCT(BlueprintType)
struct FEnemyWave
{
	GENERATED_USTRUCT_BODY()

	/**- Array of additional enemies possible in wave
	 *
	 * (Spawner randomly selects from the array in addition to lane defaults.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UEnemyBaseDA*> AdditionalEnemies;

	/**- Amount of total enemies spawned in this wave*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin=1, MakeStructureDefaultValue))
	int32 EnemyAmount = 10;
	
	/**- Spawn interval of an additional enemy in seconds*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin=0.5))
	FDoubleRange SpawnInterval;

	/**- Initial start delay of the first enemy*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin=0.5))
	double InitialDelay = 3.0;

	FEnemyWave();
};

/**Defines:
 * - waves to spawn
 * - spawning coord
 * - lane route
 */
USTRUCT(BlueprintType)
struct FEnemyLane
{
	GENERATED_USTRUCT_BODY()

	/**- Array of default enemies possible in lane
	 *
	 * Must have at least 1 air unit, if lane 'bIsGroundLane = false'. 
	 * Must have at least 1 ground unit, if lane can spawn them. 
	 * (Spawner randomly selects from the array. If you want ground only lane don't add any air unit and set 'bIsGroundLane' to true.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UEnemyBaseDA*> DefaultEnemies;
	
	/**- Enemy waves in this lane
	 *
	 * Must have at least 1 element
	 */
	UPROPERTY(EditAnywhere)
	TArray<FEnemyWave> Waves;

	/**- Tile coord of spawner for this lane
	 * - Mandatory
	 * - Unit is in tiles
	 *
	 * (Acts as the first element of waypoints.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGrid2D SpawnerTile;

	/**- Internal points between spawner tile of this lane and the level's target tile
	 * - Optional
	 * - Unit is in tiles
	 *
	 * (If not defined, lane waypoints will be the spawner and the target coords only.)
	 */
	UPROPERTY(EditAnywhere)
	TArray<FGrid2D> Waypoints;

	/**- Generated runtime, if 'bIsGroundLane = true' for land units, based on 'Waypoints'.*/
	UPROPERTY(VisibleAnywhere)
	TArray<FGrid2D> GroundWaypoints;
	
	/**- If 'bIsGroundLane = true' a road is generated along the lane.
	 * - If set to false, ground units in waves will be discarded.
	 *
	 * (TileSet should contain road meshes set, if true.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsGroundLane = false;
};



USTRUCT(BlueprintType)
struct FSoftTileSet
{
	GENERATED_USTRUCT_BODY()

	/**Is required to load the map!
	 *Default empty tile to build on*/
    UPROPERTY(EditDefaultsOnly)
    TSoftObjectPtr<UStaticMesh> BaseTile;

	/**Target tile mesh*/
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> TargetTile;

	/**Is required to load the map if a ground lane is assigned!
	 *Straight road mesh, road dir aligned to X axis*/
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> StraightRoad;

	/**Is required to load the map if a ground lane is assigned!
	 *Corner road mesh, road exits towards -X and +Y from tile*/
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> CornerRoad;

	/**Straight river tile, river aligned to Y axis*/
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> StraightRiver;

	/**Corner river mesh, river exits towards +X and +Y from tile*/
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> CornerRiver;
	
	/**Straight road bridge, road dir aligned to X axis*/
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> BridgeTile;

	/**Other scenery tiles*/
	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UStaticMesh>> SceneryTiles;

};


USTRUCT(BlueprintType)
struct FHasTileTypes
{
	GENERATED_USTRUCT_BODY()
	/**Default empty tile to build on*/
	UPROPERTY(VisibleInstanceOnly)
	bool bBaseTile = false;

	/**Spawner static mesh*/
	UPROPERTY(VisibleInstanceOnly)
	bool bSpawnerTileMesh = false;

	/**Target tile mesh*/
	UPROPERTY(VisibleInstanceOnly)
	bool bTargetTile = false;
	
	/**Straight road mesh, road dir aligned to Y axis*/
	UPROPERTY(VisibleInstanceOnly)
	bool bStraightRoad = false;

	/**Corner road mesh, road exits towards +X and -Y from tile*/
	UPROPERTY(VisibleInstanceOnly)
	bool bCornerRoad = false;

	/**Crossroad mesh*/
	UPROPERTY(VisibleInstanceOnly)
	bool bCrossRoad = false;
	
	/**Split road mesh, straight road aligned to Y split exits towards +X from tile*/
	UPROPERTY(VisibleInstanceOnly)
	bool bSplitRoad = false;
	
	/**Straight river tile, river aligned to Y axis*/
	UPROPERTY()
	bool bStraightRiver = false;

	/**Corner river mesh, river exits towards +X and -Y from tile*/
	UPROPERTY()
	bool bCornerRiver = false;
	
	/**Straight road bridge, road dir aligned to X axis*/
	UPROPERTY()
	bool bBridgeTile = false;

	/**Other scenery tiles*/
	UPROPERTY(VisibleInstanceOnly)
	bool bSceneryTiles = false;

	/**Border tile mesh*/
	UPROPERTY(VisibleInstanceOnly)
	bool bBorderTile = false;
	
};

USTRUCT()
struct FHeader
{
	GENERATED_BODY()
	
};

USTRUCT()
struct FObjectPool
{
	GENERATED_BODY()

	/**
	 *@warning Do not use this, as it won't have the name set.
	 *Use FObjectPool(const TSubclassOf<AActor> ItemClass)
	 */
	FObjectPool();

	//Assigned by constructor based on provided item's class
	UPROPERTY(VisibleAnywhere)
	FName Name;

	//Pool array for unused items
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> AvailablePool;

	UPROPERTY(VisibleAnywhere)
	TArray<int32> LastRemovedItemAmounts;

	UPROPERTY(VisibleAnywhere)
	int32 ExtraReserve = 0;
	
	explicit FObjectPool(const TSubclassOf<AActor> ItemClass);

	//Returns true if has at least one item
	bool HasAvailableItem() const;

	//Returns true if pool is class of 'ItemClass'
	bool IsClassOf(TSubclassOf<AActor> ItemClass) const;

	//Adds item back to pool
	void AddItem(AActor* Item);

	void CalculateReserve(int32 CurrentRemovedAmount);
	
};


