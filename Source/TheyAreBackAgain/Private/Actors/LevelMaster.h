// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GI_TABA.h"
#include "DataObjects/LevelDA.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SpawnerInterface.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"

#include "LevelMaster.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartNextWave, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndMatch, bool, bIsPlayerWon);

UCLASS()
class THEYAREBACKAGAIN_API ALevelMaster : public AActor, public ISpawnerInterface
{
	GENERATED_BODY()

public:

	/**Actors add delegates to know about next wave start*/
	UPROPERTY(BlueprintAssignable)
	FStartNextWave OnNextWaveStarted;

	/**Actors add delegates to know about match ended*/
	UPROPERTY(BlueprintAssignable)
	FEndMatch OnMatchEnded;
	
	/**Call this to start the next wave*/
	UFUNCTION(BlueprintCallable)
	void StartNextWave(int32 WaveIndex);

	/**Call this to end the current match*/
	UFUNCTION(BlueprintCallable)
	void EndMatch(bool bIsPlayerWon);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="State", AdvancedDisplay)
	bool bResetLevel = false;
	
	// - Add level data asset to be loaded
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Level setting")
	ULevelDA* LevelDataAsset;
	
	// - Add default tile set to be loaded
	// - Can be overridden by 'LevelDataAsset', if it feeds a valid tile set
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Level setting")
	UTileSetDA* DefaultTileSet;

	// Preload given amount of each possible enemy actors on level into objectpool
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Level setting")
	int32 EnemyReserveAmountInPool = 1;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Debug", AdvancedDisplay)
	bool bShouldLevelReload = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="State", AdvancedDisplay)
	int32 TotalEnemiesLeft = 0;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="State", AdvancedDisplay)
	TArray<int32> CurrentWaveEnemiesLeft;
		
	// - Last wave, when exceeded endmatch gets called
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="State", AdvancedDisplay)
	int32 LastWave;
		
	// - Current wave to broadcast
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="State", AdvancedDisplay)
	int32 CurrentWave;
private:
	// - Tag, that gets added to ISM components if they're buildable
	UPROPERTY(VisibleAnywhere, Category="State", AdvancedDisplay)
	FName BuildableTag = "Buildable";

	// - Found game instance
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	UGI_TABA* GameInstance;

	// - Object pool manager component
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	UObjectPoolManagerComponent* PoolManager;

	// - Currently loaded UTileSet data asset
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	UTileSetDA* LoadedTileSetAsset;

	// - Currently loaded level global air enemy asset
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	TArray<UEnemyBaseDA*> LoadedAirEnemies;

	// - Currently loaded level global ground enemy asset
	UPROPERTY(Transient, VisibleAnywhere, Category="State")
	TArray<UEnemyBaseDA*> LoadedGroundEnemies;
	
	// - reflector for success of each tile references assigned
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	FHasTileTypes TileTypesAssigned;

	// - true if set up lanes seems fine
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	bool bLaneSetupSuccess = false;

	// - true if any lane has 'bIsGroundLane = true'
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	bool bIsAnyGroundLane = false;

	// - Container for current map size set in OnLevelDataLoaded()
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	FGrid2D CurrentMapSize;
	
	// - Container for current target coord  set in OnLevelDataLoaded()
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	FGrid2D CurrentTargetCoord;

	
	// - Keep the loaded pointers in a handle, to decide if a tile set reload is neccessary or not
	// - Prevents unloading when async load's delegate finished task
	TSharedPtr<FStreamableHandle> TileSetHandle;
	// - Handle for currently loaded level data asset
	TSharedPtr<FStreamableHandle> LevelAssetHandle;
	// - Handle for currently loaded enemy data assets
	TSharedPtr<FStreamableHandle> EnemiesAssetHandle;
	// - Handle for currently loaded enemy meshes
	TSharedPtr<FStreamableHandle> EnemyMeshesHandle;

	// - Timer handle to check if assets are loaded into memory
	UPROPERTY()
	FTimerHandle WaitForAssetsLoadedHandle;

	UPROPERTY(EditAnywhere, Category="Level setting")
	FRandomStream InitialSeed = FRandomStream(21);
	
	// True if the asset is loaded in memory
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	bool bIsLevelDataLoaded = false;

	// True if the asset is loaded in memory
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	bool bIsTileSetLoaded = false;

	// True if the asset is loaded in memory
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay)
	bool bIsEnemiesLoaded = false;

	// True if all tiles should be rendered already
	UPROPERTY(Transient, VisibleAnywhere, Category="State", AdvancedDisplay, BlueprintGetter=GetIsLevelRendered)
	bool bIsLevelRendered = false;

	
public:
	ALevelMaster();

	UFUNCTION(BlueprintCallable)
	void InitMapWithDataAsset(ULevelDA* DataAssetToLoad);

	UFUNCTION(BlueprintGetter)
	bool GetIsLevelRendered();
	
protected:
	virtual void BeginPlay() override;

private:

	//Validate 'LevelDataAsset' 
	// Start map loading process
	void InitNewMap();

	/**Clears level
	 * Destroys components and pooled items
	 */
	void FlushLevelComponents();
	
	//Initiate an async load of level data asset.
	//Delegates to OnLevelDataLoaded() on load finished.
	//Called by InitNewMap()
	void LoadLevelData();

	//Resets state booleans reflecting loaded assets
	void ResetLoadedState();

	//Called on async load finished initiated from LoadLevelData().
	//Calls SetupLanes() to pre-check default requirements.
	//Initiates async load of tile set data asset if not loaded yet, or a different is requested delegates to OnTileSetLoaded(). 
	void OnLevelDataLoaded();

	//Initiates async load for valid tiles references.
	//Delegates to OnTilesLoaded() on load finished.
	//Called by OnLevelDataLoaded()
	void OnTileSetLoaded();
		
	// Checks if pointers are valid and sets bools in 'TileTypesAssigned'. 
	// Returns true if at least the base tile is valid.
	// Called by OnTileSetLoaded()
	bool ValidateTileMeshPointers();

	UPROPERTY()
	FHasTileTypes ShouldPathBeValidForTiles;
	bool bForceTryToLoadTiles = false;
	bool IsPathSeemsVaild(const FString& AssetPath);
	void TryValidateTilePointers();
	int32 InvalidLoadRetries = 0;
	void LoadInvalidPath(const FString& AssetPath);
	
	//Called on async load finished initiated from OnTileSetLoaded().
	void OnTilesLoaded();

	//Check lanes. 
	//Returns false if no lanes set. 
	//Returns false if any lane misses default enemies set.
	//Additionally adds lane spawner and map target coords to waypoints.
	//Called by OnLevelDataLoaded().
	bool SetupLanes();

	//Initiates async load for valid enemy references. 
	//Delegates to OnEnemiesLoaded() on load finished.
	//Called by OnLevelDataLoaded()
	void LoadEnemies();

	void OnEnemyDataAssetsLoaded();
	
	//Called on async load finished called from LoadEnemies().
	void OnEnemiesLoaded();

	//If loaded state booleans are true, generate level
	void CheckAllAssetsLoaded();
	
	
	/**Spawns the different tile types.
	 * Fills empty coords with base tile or scenery tile
	 * @param SceneryRatio chance to spawn scenery tile (between 0 and 1)
	 */
	void FillMap(double SceneryRatio = 0.0);
	void SpawnTargetTile();
	void SpawnSpawners();
	void SpawnBaseTiles(double SceneryRatio = 0.0);
	void SpawnBorder();

	
	/**Spawns a tile
	 * @param AtGrid map coord
	 * @param ParentComp ISM component in LevelMaster
	 * @param Direction determines tile yaw (Up = 0, Right = 90, Down = 180, Left = 270)
	 * 
	 */
	void SpawnTile(const FGrid2D AtGrid, UInstancedStaticMeshComponent* ParentComp, const EForwardDirection Direction = Up) const;
	void SpawnTile(const FTransform Transform, UInstancedStaticMeshComponent* ParentComp);

	/**Finds Instanced Static Mesh Component for the Mesh
	 * @param Mesh mesh to look for
	 * @param bIsBuildable if true, player can build on it
	 * @param TileType if set, adds enum as a tag to component on creation
	 */
	UInstancedStaticMeshComponent* FindTileParentComponent(UStaticMesh* Mesh, const bool bIsBuildable = false, const ETileType TileType = ETileType::Undefined);

	/**Creates Instanced Static Mesh Component for the Mesh, marks if buildable
	 * @param Mesh mesh to create component for
	 * @param bIsBuildable if true, player can build on it
	 *
	 * @warning Don't use this directly, use find instead
	 */
	UInstancedStaticMeshComponent* RequestNewParentComponent(UStaticMesh* Mesh, const bool bIsBuildable = false, const ETileType TileType = ETileType::Undefined);

	/**
	 * @return ISM component name, appends _b if buildable
	 */
	static FName GetParentName(FString MeshName, const bool bIsBuildable = false);

	/**Feed in a tile type enum to get a mesh pointer*/
	UStaticMesh* GetMeshFromTileType(ETileType TileType = Base) const;

	/**Gets a spawned tile instance's type and direction
	 * @param Instance Parent ISMC of tile
	 * @param Index instance index in ISMC
	 * @param OutTileType return ref for type
	 * @param OutDirection return ref for direction
	 */
	static void GetTileTypeAndDirection(const UInstancedStaticMeshComponent* Instance, const int32 Index, ETileType& OutTileType, EForwardDirection& OutDirection);

public:

	/**Getter for loaded assets
	 * @param bIsGround if true returns ground, if false then air enemy array
	 */
	UFUNCTION(BlueprintCallable)
	TArray<UEnemyBaseDA*> GetLoadedEnemies(const bool bIsGround);

	
	/**Gets the given lane data and returns true if valid*/
	bool GetWave(const int32 LaneIndex, const int32 WaveIndex, FEnemyWave& OutWave) const;
	
	/**Function for creating road at position.
	 * Merges with existing road and creates crossings.
	 * @param AtGrid Grid location to create
	 * @param RoadType Road type to create
	 * @param ForwardDirection Direction of the new road
	 * @param Requestor Name of requester, debug purposes only
	 */
	virtual void RequestRoad_I_Implementation(const FGrid2D AtGrid, const ETileType RoadType, const EForwardDirection ForwardDirection, const FName Requestor) override;

	/**Gets a random bool from initial seed. Weighted.
	 * @param Weight Chance to return true. Between 0 and 1. 0 - always false, 1 - always true.
	 */
	virtual bool RequestRandomBool_I_Implementation(const double Weight = 0.5) override;

	/**Gets a random int between Min and Max - 1.
	 * @param Max (Exclusive)
	 * @param Min optional, default to 0
	 * @warning Max param is first
	 * @return Random int32 between Min(Inclusive) and Max(Exclusive)
	 */
	virtual int32 RequestRandomRangeInt_I_Implementation(const int32 Max, const int32 Min = 0) override;

	virtual double RequestRandomFloatRange_I_Implementation(const FDoubleRange Range) override;

	UFUNCTION()
	void OnMeshOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void EnemyDied_I_Implementation() override;
};



