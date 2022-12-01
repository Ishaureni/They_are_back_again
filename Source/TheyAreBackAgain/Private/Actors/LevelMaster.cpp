// Copyright Ónodi Kristóf. All Rights Reserved.


#include "LevelMaster.h"


#include "EngineUtils.h"
#include "Interfaces/EnemyInterface.h"
#include "Actors/EnemySpawner.h"
#include "Libraries/GridHelperLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Interfaces/LevelManagerInterface.h"

ALevelMaster::ALevelMaster()
{
	PrimaryActorTick.bCanEverTick = false;
}


void ALevelMaster::StartNextWave(int32 WaveIndex)
{
	if(LastWave < CurrentWave)
	{
		EndMatch(true);
	}
	OnNextWaveStarted.Broadcast(CurrentWave);
	UE_LOG(TABA_Logger, Display, TEXT("%s: Starting next wave (%d out of %d) with %d possible spawners."), *GetActorLabel(), CurrentWave, LastWave, OnNextWaveStarted.GetAllObjects().Num());
	//CurrentWave++;
}

void ALevelMaster::EndMatch(bool bIsPlayerWon)
{
	bResetLevel = true;
	OnMatchEnded.Broadcast(bIsPlayerWon);
}

void ALevelMaster::InitMapWithDataAsset(ULevelDA* DataAssetToLoad)
{
	FlushLevelComponents();
	if(!IsValid(DataAssetToLoad))
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: No valid level data asset found to load."), *GetActorLabel());
		return;
	}
	if(LevelAssetHandle.IsValid())LevelAssetHandle->ReleaseHandle();
	LevelDataAsset = DataAssetToLoad;
	UE_LOG(TABA_Logger, Display, TEXT("%s: Loading level started."), *GetActorLabel());
	
	InitNewMap();
	ResetLoadedState();

	GetWorldTimerManager().SetTimer(WaitForAssetsLoadedHandle,this, &ALevelMaster::CheckAllAssetsLoaded, 1.0, true, 1.0);
}

void ALevelMaster::ResetLoadedState()
{
	OnNextWaveStarted.Clear();
	CurrentWaveEnemiesLeft.Empty();
	TotalEnemiesLeft = 0;
	bIsLevelDataLoaded = false;
	bIsTileSetLoaded = false;
	bIsEnemiesLoaded = false;
	bIsLevelRendered = false;
	CurrentWave = 0;
	LastWave = 0;
}

bool ALevelMaster::GetIsLevelRendered()
{
	return  bIsLevelRendered;
}

void ALevelMaster::BeginPlay()
{
	Super::BeginPlay();
	PoolManager = Cast<UObjectPoolManagerComponent>(this->GetComponentByClass(UObjectPoolManagerComponent::StaticClass()));
	GameInstance = Cast<UGI_TABA>(GetGameInstance());
	RootComponent->SetMobility(EComponentMobility::Static);
	
}

void ALevelMaster::InitNewMap()
{

	//Check if a map change should be made, release handle if needed
	//Validate 'LevelDataAsset'
	//Reset loaded states
	//Start level data async load
	if(LevelAssetHandle.IsValid())
	{
		if(LevelAssetHandle->IsActive())
		{
			if(LevelAssetHandle->GetLoadedAsset()->GetName() == LevelDataAsset->GetName())
			{
				UE_LOG(TABA_Logger, Warning, TEXT("%s: LoadLevelData() tried to load the same 'LevelDataAsset'"), *GetActorLabel());
				return;
			}
		}
		else LevelAssetHandle->ReleaseHandle();
	}
	if(!IsValid(LevelDataAsset))
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'LevelDataAsset' seems to be invalid, check it in order to run the level"), *GetActorLabel());
		return;
	}

	
	LoadLevelData();
}

void ALevelMaster::FlushLevelComponents()
{
	TArray<UActorComponent*> DumpComponentsArray;
	if(bResetLevel)
	{
		for(UActorComponent* c:this->GetComponents())
		{
			if(Cast<UInstancedStaticMeshComponent>(c))
			{
				Cast<UInstancedStaticMeshComponent>(c)->ClearInstances();

				DumpComponentsArray.Add(c);
			}
		}
	}
	for(int i = DumpComponentsArray.Num(); i > 0; --i)
	{
		DumpComponentsArray[i-1]->DestroyComponent();
	}
}

void ALevelMaster::LoadLevelData()
{
	LevelAssetHandle = GameInstance->StreamableManager.RequestAsyncLoad(LevelDataAsset, FStreamableDelegate::CreateUObject(this, &ALevelMaster::OnLevelDataLoaded));
}


void ALevelMaster::OnLevelDataLoaded()
{
	//Validate handle
	if(!LevelAssetHandle.IsValid() || !IsValid(LevelAssetHandle->GetLoadedAsset()))
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'LevelAssetHandle' couldn't store loaded level asset ptr"), *GetActorLabel());
		return;
	}
	if(!IsValid(LevelDataAsset))
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'LevelDataAsset' seems to be invalid, check it in order to run the level"), *GetActorLabel());
		return;
	}

	//Mark level loaded
	bIsLevelDataLoaded = true;
	UE_LOG(TABA_Logger, Display, TEXT("%s: 'LevelDataAsset' loaded."), *GetActorLabel());

	InitialSeed.Initialize(LevelDataAsset->InitialSeed);
	//Check lanes
	bLaneSetupSuccess = SetupLanes();
	if(!bLaneSetupSuccess)
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'LevelDataAsset' has lane problems, output log should have further info"), *GetActorLabel());
		return;
	}
	UE_LOG(TABA_Logger, Display, TEXT("%s: Lanes initialized."), *GetActorLabel());

	//Set map size, and validate target coord
	CurrentMapSize = FGrid2D(LevelDataAsset->N, LevelDataAsset->M);
	CurrentTargetCoord = LevelDataAsset->TargetCoord.Clamp(FGrid2D::Zero(), FGrid2D(LevelDataAsset->N-1, LevelDataAsset->M-1));
	if(!CurrentTargetCoord.Equals(LevelDataAsset->TargetCoord))
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: TargetCoord was out of bounds, clamped to map area"), *GetActorLabel());
	}

	//Validate tile set, use default upon missing, or purposely not being assigned in data asset
	UTileSetDA* TileSetToLoad;
	if(IsValid(LevelDataAsset->TileSet))
	{
		TileSetToLoad = LevelDataAsset->TileSet;
		UE_LOG(TABA_Logger, Display, TEXT("%s: TileSetToLoad uses level data asset's set"), *GetActorLabel());
	}
	else if(IsValid(DefaultTileSet))
	{
		TileSetToLoad = DefaultTileSet;
		UE_LOG(TABA_Logger, Display, TEXT("%s: TileSetToLoad uses default set"), *GetActorLabel());
	}
	else
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: No valid TileSet found, check it in order to run the level"), *GetActorLabel());
		return;
	}
	if(TileSetHandle.IsValid() && TileSetHandle->IsActive())
	{
		if(TileSetHandle->GetLoadedAsset()->GetName() != TileSetToLoad->GetName())
		{
			TileSetHandle->ReleaseHandle();
			UE_LOG(TABA_Logger, Display, TEXT("%s: Tile set handle released for switching to an other set"), *GetActorLabel());
		}
		else
		{
			UE_LOG(TABA_Logger, Display, TEXT("%s: Tile set unchanged, reusing"), *GetActorLabel());
			bIsTileSetLoaded = true;
		}
	}

	//If tile set not loaded yet, or was released due to change, initiate async load 
	if(!TileSetHandle.IsValid() || !TileSetHandle->IsActive())
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Tile Async load started, loaded asset shared pointer assigned to TileSetHandle"), *GetActorLabel());
		TileSetHandle = GameInstance->StreamableManager.RequestAsyncLoad(TileSetToLoad, FStreamableDelegate::CreateUObject(this, &ALevelMaster::OnTileSetLoaded), FStreamableManager::DefaultAsyncLoadPriority, true);
	}
	
	LoadEnemies();
}

void ALevelMaster::OnTileSetLoaded()
{
	
	LoadedTileSetAsset = Cast<UTileSetDA>(TileSetHandle->GetLoadedAsset());
	
	//Validate tile set loaded in memory
	if(!IsValid(LoadedTileSetAsset))
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'LoadedTileSetAsset' seems to be invalid, check it in order to run the level"), *GetActorLabel());
		return;
	}
	
	//Validate tile mesh pointers in set
	if(!ValidateTileMeshPointers() && !bForceTryToLoadTiles)
	{
		TryValidateTilePointers();
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'ValidateTileMeshPointers' failed, no base tile found check it in order to run the level"), *GetActorLabel());
		
		return;
	}
	if(bForceTryToLoadTiles) UE_LOG(TABA_Logger, Warning, TEXT("%s: Trying to force load minimal tiles."), *GetActorLabel());

	UE_LOG(TABA_Logger, Display, TEXT("%s: Tile mesh pointer validation succeded."), *GetActorLabel());

	//Get mesh pointers and initiate async load
	TArray<FSoftObjectPath> TilesToLoad;
	if(TileTypesAssigned.bBaseTile)TilesToLoad.AddUnique(LoadedTileSetAsset->BaseTileMeshPtr.ToString());
	if(TileTypesAssigned.bTargetTile)TilesToLoad.AddUnique(LoadedTileSetAsset->TargetTileMeshPtr.ToString());
	if(TileTypesAssigned.bStraightRoad)
	{
		TilesToLoad.AddUnique(LoadedTileSetAsset->StraightRoadTileMeshPtr.ToString());
		if(TileTypesAssigned.bCrossRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->CrossRoadTileMeshPtr.ToString());
		if(TileTypesAssigned.bSplitRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->SplitRoadTileMeshPtr.ToString());
		if(TileTypesAssigned.bCornerRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->CornerRoadTileMeshPtr.ToString());
	}
	if(TileTypesAssigned.bSceneryTiles)
	{
		for(TSoftObjectPtr<UStaticMesh> SO: LoadedTileSetAsset->SceneryTileMeshPtrs)
		{
			if(SO != nullptr)
			{
				TilesToLoad.AddUnique(SO.ToString());
			}
		}
	}
	if(TileTypesAssigned.bSpawnerTileMesh)TilesToLoad.AddUnique(LoadedTileSetAsset->SpawnerTileMeshPtr.ToString());
	if(TileTypesAssigned.bBorderTile)TilesToLoad.AddUnique(LoadedTileSetAsset->LevelBorderTileMeshPtr.ToString());

	if(TilesToLoad.Num()>0)
	{
		if(GameInstance==nullptr)
		{
			UE_LOG(TABA_Logger, Error, TEXT("%s: GameInstance is not assigned."), *GetActorLabel());
			return;
		}
		UE_LOG(TABA_Logger, Display, TEXT("%s: Loading %d tiles."), *GetActorLabel(), TilesToLoad.Num());

		//RequestAsyncLoad objects for spawning tiles in delegate, they get unloaded after delegate ran
		GameInstance->StreamableManager.RequestAsyncLoad(TilesToLoad, FStreamableDelegate::CreateUObject(this, &ALevelMaster::OnTilesLoaded));
	}
	else
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: No path found to load with StreamableManager"), *GetActorLabel());
	}
}

bool ALevelMaster::ValidateTileMeshPointers()
{
	UE_LOG(TABA_Logger, Display, TEXT("%s: Validating tile pointers."), *GetActorLabel());

	TileTypesAssigned.bBaseTile = false;
	TileTypesAssigned.bTargetTile = false;
	TileTypesAssigned.bStraightRoad = false;
	TileTypesAssigned.bCornerRoad = false;
	TileTypesAssigned.bCrossRoad = false;
	TileTypesAssigned.bSplitRoad = false;
	TileTypesAssigned.bSceneryTiles = false;
	TileTypesAssigned.bSpawnerTileMesh = false;
	TileTypesAssigned.bBorderTile = false;
	
	TileTypesAssigned.bBaseTile = LoadedTileSetAsset->BaseTileMeshPtr.IsValid();
	TileTypesAssigned.bTargetTile = LoadedTileSetAsset->TargetTileMeshPtr.IsValid();
	TileTypesAssigned.bStraightRoad = LoadedTileSetAsset->StraightRoadTileMeshPtr.IsValid();
	if(TileTypesAssigned.bStraightRoad)
	{
		TileTypesAssigned.bCornerRoad = LoadedTileSetAsset->CornerRoadTileMeshPtr.IsValid();
		TileTypesAssigned.bCrossRoad = LoadedTileSetAsset->CrossRoadTileMeshPtr.IsValid();
		TileTypesAssigned.bSplitRoad = LoadedTileSetAsset->SplitRoadTileMeshPtr.IsValid();
	}
	if(LoadedTileSetAsset->SceneryTileMeshPtrs.Num()>0)
	{
		for(TSoftObjectPtr<UStaticMesh> SO: LoadedTileSetAsset->SceneryTileMeshPtrs)
		{
			if(SO.IsValid())
			{
				TileTypesAssigned.bSceneryTiles = true;
				break;
			}
		}
	}
	TileTypesAssigned.bSpawnerTileMesh = LoadedTileSetAsset->SpawnerTileMeshPtr.IsValid();
	TileTypesAssigned.bBorderTile = LoadedTileSetAsset->LevelBorderTileMeshPtr.IsValid();

	
	ShouldPathBeValidForTiles.bBaseTile = IsPathSeemsVaild(LoadedTileSetAsset->BaseTileMeshPtr.ToString());
	ShouldPathBeValidForTiles.bTargetTile = IsPathSeemsVaild(LoadedTileSetAsset->TargetTileMeshPtr.ToString());
	ShouldPathBeValidForTiles.bStraightRoad = IsPathSeemsVaild(LoadedTileSetAsset->StraightRoadTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bStraightRoad)
	{
		ShouldPathBeValidForTiles.bCornerRoad = IsPathSeemsVaild(LoadedTileSetAsset->CornerRoadTileMeshPtr.ToString());
		ShouldPathBeValidForTiles.bCrossRoad = IsPathSeemsVaild(LoadedTileSetAsset->CrossRoadTileMeshPtr.ToString());
		ShouldPathBeValidForTiles.bSplitRoad = IsPathSeemsVaild(LoadedTileSetAsset->SplitRoadTileMeshPtr.ToString());
	}
	if(LoadedTileSetAsset->SceneryTileMeshPtrs.Num()>0)
	{
		for(TSoftObjectPtr<UStaticMesh> SO: LoadedTileSetAsset->SceneryTileMeshPtrs)
		{
			if(IsPathSeemsVaild(SO.ToString()))
			{
				ShouldPathBeValidForTiles.bSceneryTiles = true;
				break;
			}
		}
	}
	ShouldPathBeValidForTiles.bSpawnerTileMesh = IsPathSeemsVaild(LoadedTileSetAsset->SpawnerTileMeshPtr.ToString());
	ShouldPathBeValidForTiles.bBorderTile = IsPathSeemsVaild(LoadedTileSetAsset->LevelBorderTileMeshPtr.ToString());
	
	UE_LOG(TABA_Logger, Display, TEXT("%s: Base tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bBaseTile ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->BaseTileMeshPtr.ToString());
	UE_LOG(TABA_Logger, Display, TEXT("%s: Target tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bTargetTile ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->TargetTileMeshPtr.ToString());
	UE_LOG(TABA_Logger, Display, TEXT("%s: Straight road tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bStraightRoad ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->StraightRoadTileMeshPtr.ToString());
	UE_LOG(TABA_Logger, Display, TEXT("%s: Corner road tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bCornerRoad ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->CornerRoadTileMeshPtr.ToString());
	UE_LOG(TABA_Logger, Display, TEXT("%s: Split road tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bSplitRoad ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->SplitRoadTileMeshPtr.ToString());
	UE_LOG(TABA_Logger, Display, TEXT("%s: Cross road tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bCrossRoad ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->CrossRoadTileMeshPtr.ToString());
	for(TSoftObjectPtr<UStaticMesh> SO: LoadedTileSetAsset->SceneryTileMeshPtrs)
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Scenery tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bSceneryTiles ? TEXT("succeded") : TEXT("failed"), *SO.ToString());
	}

	UE_LOG(TABA_Logger, Display, TEXT("%s: Spawner tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bSpawnerTileMesh ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->SpawnerTileMeshPtr.ToString());
	UE_LOG(TABA_Logger, Display, TEXT("%s: Border tile validation %s. Path: %s"), *GetActorLabel(), TileTypesAssigned.bBorderTile ? TEXT("succeded") : TEXT("failed"), *LoadedTileSetAsset->LevelBorderTileMeshPtr.ToString());

	bool bAllShouldBeValid = true;
	if(ShouldPathBeValidForTiles.bBaseTile) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bTargetTile) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bStraightRoad) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bCornerRoad) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bCrossRoad) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bSplitRoad) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bSceneryTiles) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bSceneryTiles;
	if(ShouldPathBeValidForTiles.bSpawnerTileMesh) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	if(ShouldPathBeValidForTiles.bBorderTile) bAllShouldBeValid = bAllShouldBeValid && TileTypesAssigned.bBaseTile;
	
	return bAllShouldBeValid;
}

bool ALevelMaster::IsPathSeemsVaild(const FString& AssetPath)
{
	return AssetPath.StartsWith("/game");
}

void ALevelMaster::TryValidateTilePointers()
{
	TArray<FString> TilesToLoad;
	if(!TileTypesAssigned.bBaseTile)TilesToLoad.AddUnique(LoadedTileSetAsset->BaseTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bTargetTile && !TileTypesAssigned.bTargetTile) TilesToLoad.AddUnique(LoadedTileSetAsset->TargetTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bStraightRoad && !TileTypesAssigned.bStraightRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->StraightRoadTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bCrossRoad && !TileTypesAssigned.bCrossRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->CrossRoadTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bSplitRoad && !TileTypesAssigned.bSplitRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->SplitRoadTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bCornerRoad && !TileTypesAssigned.bCornerRoad)TilesToLoad.AddUnique(LoadedTileSetAsset->CornerRoadTileMeshPtr.ToString());
	for(TSoftObjectPtr<UStaticMesh> SO: LoadedTileSetAsset->SceneryTileMeshPtrs)
	{
		if(ShouldPathBeValidForTiles.bSceneryTiles && !TileTypesAssigned.bSceneryTiles)	TilesToLoad.AddUnique(SO.ToString());

	}
	if(ShouldPathBeValidForTiles.bSpawnerTileMesh && !TileTypesAssigned.bSpawnerTileMesh)TilesToLoad.AddUnique(LoadedTileSetAsset->SpawnerTileMeshPtr.ToString());
	if(ShouldPathBeValidForTiles.bBorderTile && !TileTypesAssigned.bBorderTile)TilesToLoad.AddUnique(LoadedTileSetAsset->LevelBorderTileMeshPtr.ToString());
	
	for(FString s:TilesToLoad)
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: Trying to resolve static mesh path: "), *s);
		LoadInvalidPath(s);
	}
	
	FTimerHandle dummy;
	if(InvalidLoadRetries > 5)
	{
		if(TileTypesAssigned.bTargetTile)
		{
			UE_LOG(TABA_Logger, Error, TEXT("%s: Tile set only partly loaded, causing experience issues."), *GetActorLabel());
			OnTileSetLoaded();
			bForceTryToLoadTiles = true;
		}
		else
		{
			UE_LOG(TABA_Logger, Error, TEXT("%s: Tile set couldn't be loaded, terminating level."), *GetActorLabel());
		
			bShouldLevelReload = true;
			return;			
		}
	}
	else GetWorldTimerManager().SetTimer(dummy, this, &ALevelMaster::OnTileSetLoaded, 5);
}

void ALevelMaster::LoadInvalidPath(const FString& AssetPath)
{
	StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *AssetPath);
}

void ALevelMaster::OnTilesLoaded()
{
	//Set instanced static meshes for tiles
	FindTileParentComponent(LoadedTileSetAsset->BaseTileMeshPtr.Get(), true, Base);
	if(TileTypesAssigned.bTargetTile)FindTileParentComponent(LoadedTileSetAsset->TargetTileMeshPtr.Get(), false, Target);
	else FindTileParentComponent(LoadedTileSetAsset->BaseTileMeshPtr.Get());
	if(TileTypesAssigned.bStraightRoad)
	{
		FindTileParentComponent(LoadedTileSetAsset->StraightRoadTileMeshPtr.Get(), false, RoadStraight);
		if(TileTypesAssigned.bCornerRoad)FindTileParentComponent(LoadedTileSetAsset->CornerRoadTileMeshPtr.Get(), false, RoadCorner);
		if(TileTypesAssigned.bCrossRoad)FindTileParentComponent(LoadedTileSetAsset->CrossRoadTileMeshPtr.Get(), false, RoadCrossing);
		if(TileTypesAssigned.bSplitRoad)FindTileParentComponent(LoadedTileSetAsset->SplitRoadTileMeshPtr.Get(), false, RoadSplit);
	}
	else FindTileParentComponent(LoadedTileSetAsset->BaseTileMeshPtr.Get());
	if(TileTypesAssigned.bSceneryTiles)
	{
		for(int32 i = 0; i < LoadedTileSetAsset->SceneryTileMeshPtrs.Num(); ++i)
		{
			FindTileParentComponent(LoadedTileSetAsset->SceneryTileMeshPtrs[i].Get(), false, Scenery);
		}
	}
	else FindTileParentComponent(LoadedTileSetAsset->BaseTileMeshPtr.Get(), true, Base);
	if(TileTypesAssigned.bSpawnerTileMesh)FindTileParentComponent(LoadedTileSetAsset->SpawnerTileMeshPtr.Get(), false, Spawner);
	else FindTileParentComponent(LoadedTileSetAsset->BaseTileMeshPtr.Get());
	if(TileTypesAssigned.bBorderTile)FindTileParentComponent(LoadedTileSetAsset->LevelBorderTileMeshPtr.Get());
	
	bIsTileSetLoaded = true;

}

bool ALevelMaster::SetupLanes()
{
	bIsAnyGroundLane = false;
	if(LevelDataAsset->Lanes.Num()<=0)
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: 'LevelDataAsset' has no lanes, check it in order to run the level"), *GetActorLabel());
		return false;		
	}
	
	// - true if every lane has default enemies
	bool bIsAnyEnemyAssigned = false;
	
	for(int32 i = 0; i < LevelDataAsset->Lanes.Num(); ++i)
	{	
		if(!bIsAnyGroundLane)
		{
			bIsAnyGroundLane = LevelDataAsset->Lanes[i].bIsGroundLane;
		}
		for(const UEnemyBaseDA* Enemy: LevelDataAsset->Lanes[i].DefaultEnemies)
		{
			if(IsValid(Enemy))
			{
				bIsAnyEnemyAssigned = true;
				break;
			}
		}
		if(!bIsAnyEnemyAssigned)
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: 'LevelDataAsset' has a lane without default enemies"), *GetActorLabel());
		}
		LastWave = FMath::Max(LastWave, LevelDataAsset->Lanes[i].Waves.Num()-1);

		for(int32 j = 0; j < LevelDataAsset->Lanes[i].Waves.Num(); ++j)
		{
			if(CurrentWaveEnemiesLeft.Num()<=j)
			{
				CurrentWaveEnemiesLeft.Add(LevelDataAsset->Lanes[i].Waves[j].EnemyAmount);
			}
			else
			{
				CurrentWaveEnemiesLeft[j] += LevelDataAsset->Lanes[i].Waves[j].EnemyAmount;
			}
			TotalEnemiesLeft += LevelDataAsset->Lanes[i].Waves[j].EnemyAmount;
		}
	}
	UE_LOG(TABA_Logger, Display, TEXT("%s: Last wave set to %d"), *GetActorLabel(), LastWave+1);
	return true;
}

void ALevelMaster::LoadEnemies()
{
	TArray<FSoftObjectPath> EnemiesToLoad;

	for(int32 d = 0; d < LevelDataAsset->DefaultLevelEnemies.Num(); ++ d)
	{
		if(IsValid(LevelDataAsset->DefaultLevelEnemies[d])) EnemiesToLoad.AddUnique(LevelDataAsset->DefaultLevelEnemies[d]);
	}
	
	for(int32 l = 0; l < LevelDataAsset->Lanes.Num(); ++l)
	{
		for(int32 e = 0; e < LevelDataAsset->Lanes[l].DefaultEnemies.Num(); ++e)
		{
			if(IsValid(LevelDataAsset->Lanes[l].DefaultEnemies[e]))
			{
				EnemiesToLoad.AddUnique(LevelDataAsset->Lanes[l].DefaultEnemies[e]);
			}
			for(int32 w = 0; w < LevelDataAsset->Lanes[l].Waves.Num(); ++w)
			{
				for(int32 ae = 0; ae < LevelDataAsset->Lanes[l].Waves[w].AdditionalEnemies.Num(); ++ae)
				{
					EnemiesToLoad.AddUnique(LevelDataAsset->Lanes[l].Waves[w].AdditionalEnemies[ae]);
				}
			}
		}
	}
	
	if(EnemiesAssetHandle.IsValid() && EnemiesAssetHandle->IsActive())EnemiesAssetHandle->ReleaseHandle();
		
	if(EnemiesToLoad.Num()>0)
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Loading %d enemy data assets."), *GetActorLabel(), EnemiesToLoad.Num());

		EnemiesAssetHandle = GameInstance->StreamableManager.RequestAsyncLoad(EnemiesToLoad, FStreamableDelegate::CreateUObject(this, &ALevelMaster::OnEnemyDataAssetsLoaded),FStreamableManager::DefaultAsyncLoadPriority, true);
	}
	else
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: No path found to load with StreamableManager"), *GetActorLabel());
	}
}

void ALevelMaster::OnEnemyDataAssetsLoaded()
{
	if(!EnemiesAssetHandle.IsValid())
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: Enemy data asset failed to load."), *GetActorLabel());
		return;
	}
	
	TArray<FSoftObjectPath> EnemiesToLoad;
	TArray<UObject*> EnemyAssets;
	EnemiesAssetHandle->GetLoadedAssets(EnemyAssets);
	
	for(int32 i = 0; i < EnemyAssets.Num(); ++i)
	{
		const UEnemyBaseDA* CurrentDA = Cast<UEnemyBaseDA>(EnemyAssets[i]);
		if(IsValid(CurrentDA))
		{
			EnemiesToLoad.AddUnique(CurrentDA->MeshPtr.ToString());
		}
	}
	if(EnemyMeshesHandle.IsValid() && EnemyMeshesHandle->IsActive())EnemyMeshesHandle->ReleaseHandle();

	if(EnemiesToLoad.Num()>0)
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Loading %d enemy meshes."), *GetActorLabel(), EnemiesToLoad.Num());
		EnemyMeshesHandle = GameInstance->StreamableManager.RequestAsyncLoad(EnemiesToLoad, FStreamableDelegate::CreateUObject(this, &ALevelMaster::OnEnemiesLoaded),FStreamableManager::DefaultAsyncLoadPriority, true);
	}
	else
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: No path found to load with StreamableManager"), *GetActorLabel());
	}
}

void ALevelMaster::OnEnemiesLoaded()
{
	TArray<UObject*> LoadedEnemiesTemp;
	EnemiesAssetHandle->GetLoadedAssets(LoadedEnemiesTemp);
	for(UObject* Obj:LoadedEnemiesTemp)
	{
		if(!IsValid(Obj))
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: Load of '%s' enemy asset failed."), *GetActorLabel(), *Obj->GetName());
			continue;
		}
		if(Cast<UEnemyBaseDA>(Obj)->IsFlying) LoadedAirEnemies.Add(Cast<UEnemyBaseDA>(Obj));
		else LoadedGroundEnemies.Add(Cast<UEnemyBaseDA>(Obj));
		
		for(AActor* Enemy:PoolManager->PreloadItemsInPool(AEnemyBase::StaticClass(), EnemyReserveAmountInPool))
		{
			if(Cast<IEnemyInterface>(Enemy))
			{
				IEnemyInterface::Execute_InitEnemy_I(Enemy, Cast<UEnemyBaseDA>(Obj));
			}
		}
	}

	
	bIsEnemiesLoaded = true;
}

void ALevelMaster::CheckAllAssetsLoaded()
{
	if(bIsLevelDataLoaded && bIsTileSetLoaded && bIsEnemiesLoaded)
	{
		GetWorldTimerManager().ClearTimer(WaitForAssetsLoadedHandle);
		UE_LOG(TABA_Logger, Display, TEXT("%s: All Assets loaded, rendering map."), *GetActorLabel());
		FillMap(LevelDataAsset->SceneryTileChance);
	}
}

void ALevelMaster::FillMap(double SceneryRatio)
{
	SpawnTargetTile();
	SpawnSpawners();
	SpawnBaseTiles(SceneryRatio);
	SpawnBorder();
	
	bIsLevelRendered = true;
}

void ALevelMaster::SpawnTargetTile()
{
	UStaticMesh* Mesh = TileTypesAssigned.bTargetTile?LoadedTileSetAsset->TargetTileMeshPtr.Get():LoadedTileSetAsset->BaseTileMeshPtr.Get();
	UInstancedStaticMeshComponent* TargetParent = FindTileParentComponent(Mesh);
	TargetParent->SetGenerateOverlapEvents(true);
	TargetParent->SetCollisionProfileName("OverlapAllDynamic");
	TargetParent->OnComponentBeginOverlap.AddDynamic(this, &ALevelMaster::OnMeshOverlapped);
	SpawnTile(CurrentTargetCoord, TargetParent);
}

void ALevelMaster::SpawnSpawners()
{
	UStaticMesh* Mesh = TileTypesAssigned.bSpawnerTileMesh?LoadedTileSetAsset->SpawnerTileMeshPtr.Get():LoadedTileSetAsset->BaseTileMeshPtr.Get();
	for(int32 i = 0; i < LevelDataAsset->Lanes.Num(); ++i)
	{
		if(LevelDataAsset->Lanes[i].SpawnerTile.LinearDistance(CurrentTargetCoord) <= 2)
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: Spawner tile (%s) is too close (%d tiles) to target tile in lane #%d. Possibly the level will not be playable."), *GetActorLabel(), *LevelDataAsset->Lanes[i].SpawnerTile.ToString(), LevelDataAsset->Lanes[i].SpawnerTile.LinearDistance(CurrentTargetCoord), i);
		}
		FHitResult Hit;
		if(UGridHelperLibrary::IsGridOccupied(this, LevelDataAsset->Lanes[i].SpawnerTile, Hit))
		{
			UE_LOG(TABA_Logger, Error, TEXT("%s: Spawner tile (%s) in lane %d is conflicting with already existing tile: %s. Skipping lane generation."), *GetActorLabel(), *LevelDataAsset->Lanes[i].SpawnerTile.ToString(), i, *Hit.Component->GetName());
			continue;
		}
		FTransform Transform = FTransform(UGridHelperLibrary::GridPositionToWorld(LevelDataAsset->Lanes[i].SpawnerTile, this));
		
		AActor* NewSpawner = PoolManager->RequestItemFromPool(AEnemySpawner::StaticClass(), Transform);
		if(Cast<ISpawnerInterface>(NewSpawner))
		{
			Execute_InitSpawnerActor_I(NewSpawner, Transform, i, CurrentTargetCoord, LevelDataAsset->Lanes[i]);
		}
		SpawnTile(Transform, FindTileParentComponent(Mesh));
	}
}

void ALevelMaster::SpawnBaseTiles(double SceneryRatio)
{
	for(int32 i = 0; i < LevelDataAsset->N; ++i)
	{
	for(int32 j = 0; j < LevelDataAsset->M; ++j)
	{
		if(UGridHelperLibrary::IsGridOccupied(this, FGrid2D(i,j))) continue;

		
		UStaticMesh* Mesh = LoadedTileSetAsset->BaseTileMeshPtr.Get();
		bool bBuildable = true;
		ETileType TileType = Base;
		EForwardDirection Direction = Up;
		
		if(TileTypesAssigned.bSceneryTiles)
		{
			const bool bIsScenery = InitialSeed.GetFraction() < SceneryRatio;
			if(bIsScenery)
			{
				int32 Index = InitialSeed.RandRange(0, LoadedTileSetAsset->SceneryTileMeshPtrs.Num()-1);
				Mesh = LoadedTileSetAsset->SceneryTileMeshPtrs[Index].Get();
				if(!IsValid(Mesh))
				{
					UE_LOG(TABA_Logger, Error, TEXT("%s: Failed to load scenery tile mesh from index %d"), *GetActorLabel(), Index);
				}
				else
				{
					Direction = static_cast<EForwardDirection>(InitialSeed.RandRange(0, 3));
					TileType = Scenery;
					bBuildable = false;
				}
			}
		}
		if(!IsValid(Mesh))
		{
			Mesh = LoadedTileSetAsset->BaseTileMeshPtr.Get();
		}
		SpawnTile(FGrid2D(i,j), FindTileParentComponent(Mesh, bBuildable, TileType), Direction);
	}
	}
}

void ALevelMaster::SpawnBorder()
{
	if(!TileTypesAssigned.bBorderTile) return;

	UInstancedStaticMeshComponent* BorderParent = FindTileParentComponent(LoadedTileSetAsset->LevelBorderTileMeshPtr.Get());

	for(int32 i = -1; i <= LevelDataAsset->N; ++i)
	{
		SpawnTile(FGrid2D(i, -1), BorderParent);
		SpawnTile(FGrid2D(i, LevelDataAsset->M), BorderParent);

		if(i == -1 || i == LevelDataAsset->M)
		{
			for(int32 j = 0; j < LevelDataAsset->M; ++j)
			{
				SpawnTile(FGrid2D(-1, j), BorderParent);
				SpawnTile(FGrid2D(LevelDataAsset->N, j), BorderParent);
			}
		}
	}
}


void ALevelMaster::SpawnTile(const FGrid2D AtGrid, UInstancedStaticMeshComponent* ParentComp, const EForwardDirection Direction) const
{
	ParentComp->AddInstance(FTransform(FRotator(0.0,Direction*90.0,0.0),UGridHelperLibrary::GridPositionToWorld(AtGrid, this), FVector::OneVector*UGridHelperLibrary::GetGridGlobalScale(this)), true);
}
void ALevelMaster::SpawnTile(const FTransform Transform, UInstancedStaticMeshComponent* ParentComp)
{
	FTransform NewTransform = Transform;
	NewTransform.SetScale3D(FVector::OneVector*UGridHelperLibrary::GetGridGlobalScale(this));
	ParentComp->AddInstance(NewTransform, true);
}

UInstancedStaticMeshComponent* ALevelMaster::FindTileParentComponent(UStaticMesh* Mesh, const bool bIsBuildable, const ETileType TileType)
{
	const FName CompName = GetParentName(Mesh->GetName(), bIsBuildable);
	
	TArray<UActorComponent*> Comps = this->GetComponentsByTag(UInstancedStaticMeshComponent::StaticClass(), CompName);
	if(Comps.Num()>0)
	{
		for(UActorComponent* C:Comps)
		{
			if (bIsBuildable && C->ComponentHasTag(BuildableTag) || !bIsBuildable && !C->ComponentHasTag(BuildableTag))
			{
				return Cast<UInstancedStaticMeshComponent>(C);
			}
		}
	}
	//No ISM found, create a new one
	else
	{
		return RequestNewParentComponent(Mesh, bIsBuildable, TileType);
	}
	
	UE_LOG(TABA_Logger, Error, TEXT("%s: No instanced static mesh component found with name: %s"), *GetActorLabel(), *CompName.ToString());
	return nullptr;
}

UInstancedStaticMeshComponent* ALevelMaster::RequestNewParentComponent(UStaticMesh* Mesh, const bool bIsBuildable, const ETileType TileType)
{
	const FName CompName = GetParentName(Mesh->GetName(), bIsBuildable);

	//Create and init a new ISM, with Mesh and Tag set
	UInstancedStaticMeshComponent* NewComp = NewObject<UInstancedStaticMeshComponent>(this, CompName);
	
	if(ensureMsgf(NewComp, TEXT("%s::RequestNewISMC() - NewObject() failed to create ISM"), *GetActorLabel()))
	{
		NewComp->RegisterComponent();
		NewComp->AttachToComponent(this->RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
		NewComp->ComponentTags.Add(CompName);
		if(bIsBuildable)NewComp->ComponentTags.Add(BuildableTag);
		if(TileType != Undefined) NewComp->ComponentTags.Add(UEnum::GetValueAsName(TileType));
		this->AddInstanceComponent(NewComp);
		NewComp->SetStaticMesh(Mesh);
		NewComp->SetMobility(EComponentMobility::Static);
		NewComp->SetCollisionProfileName("TilePreset");
		NewComp->NumCustomDataFloats = 1;
	
		UE_LOG(TABA_Logger, Display, TEXT("%s: Instanced static mesh component created: %s"), *GetActorLabel(), *CompName.ToString());

		return NewComp;
	}

	UE_LOG(TABA_Logger, Error, TEXT("%s: Failed to create ISMC: %s"), *GetActorLabel(), *CompName.ToString());

	return nullptr;
}

FName ALevelMaster::GetParentName(FString MeshName, const bool bIsBuildable)
{
	if(bIsBuildable) MeshName += "_b";
	return FName(MeshName);
}

UStaticMesh* ALevelMaster::GetMeshFromTileType(const ETileType TileType) const
{
	UStaticMesh* ReturnMesh = nullptr;
	switch (TileType)
	{
	case Base:
		ReturnMesh = LoadedTileSetAsset->BaseTileMeshPtr.Get();
		break;
	case Target:
		if(TileTypesAssigned.bTargetTile) ReturnMesh = LoadedTileSetAsset->TargetTileMeshPtr.Get();
		break;
	case Spawner:
		if(TileTypesAssigned.bSpawnerTileMesh) ReturnMesh = LoadedTileSetAsset->SpawnerTileMeshPtr.Get();
		break;
	case RoadStraight:
		if(TileTypesAssigned.bStraightRoad) ReturnMesh = LoadedTileSetAsset->StraightRoadTileMeshPtr.Get();
		break;
	case RoadCorner:
		if(TileTypesAssigned.bCornerRoad) ReturnMesh = LoadedTileSetAsset->CornerRoadTileMeshPtr.Get();
		break;
	case RoadSplit:
		if(TileTypesAssigned.bSplitRoad) ReturnMesh = LoadedTileSetAsset->SplitRoadTileMeshPtr.Get();
		break;
	case RoadCrossing:
		if(TileTypesAssigned.bCrossRoad) ReturnMesh = LoadedTileSetAsset->CrossRoadTileMeshPtr.Get();
		break;
	case Scenery:
		if(TileTypesAssigned.bSceneryTiles)
		{
			const int32 RandomIndex = InitialSeed.RandRange(0, LoadedTileSetAsset->SceneryTileMeshPtrs.Num());
			ReturnMesh = LoadedTileSetAsset->SceneryTileMeshPtrs[RandomIndex].Get();
		}
		break;
	default:	ReturnMesh = LoadedTileSetAsset->BaseTileMeshPtr.Get();
		break;
	}
	if (!IsValid(ReturnMesh))ReturnMesh = LoadedTileSetAsset->BaseTileMeshPtr.Get();
	return ReturnMesh;
}

void ALevelMaster::GetTileTypeAndDirection(const UInstancedStaticMeshComponent* Instance, const int32 Index,
	ETileType& OutTileType, EForwardDirection& OutDirection)
{
	FTransform Transform;
	Instance->GetInstanceTransform(Index, Transform);

	int32 Yaw = Transform.Rotator().Yaw;
	switch (Yaw)
	{
	case 90:
		OutDirection = Right;
		break;
	case 180:
		OutDirection = Down;
		break;
	case 270:
		OutDirection = Left;
		break;
	default:
		OutDirection = Up;
		break;
	}

	const TArray<FName> Tags = Instance->ComponentTags;
	for(const ETileType Tile:TEnumRange<ETileType>())
	{
		if(Tags.Contains(UEnum::GetValueAsName(Tile)))
		{
			OutTileType = Tile;
			break;
		}
	}	
}

TArray<UEnemyBaseDA*> ALevelMaster::GetLoadedEnemies(const bool bIsGround)
{
	return bIsGround ? LoadedGroundEnemies : LoadedAirEnemies;
}

bool ALevelMaster::GetWave(const int32 LaneIndex, const int32 WaveIndex, FEnemyWave& OutWave) const
{

	const bool bHasWave = LevelDataAsset->Lanes[LaneIndex].Waves.Num() > WaveIndex;
	if(bHasWave) OutWave = LevelDataAsset->Lanes[LaneIndex].Waves[WaveIndex];
	return bHasWave;
}

void ALevelMaster::RequestRoad_I_Implementation(const FGrid2D AtGrid, const ETileType RoadType, const EForwardDirection ForwardDirection, const FName Requestor)
{
	ISpawnerInterface::RequestRoad_I_Implementation(AtGrid, RoadType, ForwardDirection, Requestor);

	EForwardDirection NewDir = ForwardDirection;
	ETileType NewType = RoadType;
	
	//Tile already exists at location
	if(FHitResult HitResult; UGridHelperLibrary::IsGridOccupied(this, AtGrid, HitResult))
	{
		if(!IsValid(HitResult.GetActor()))
		{
			UE_LOG(LogTemp, Warning, TEXT("HitResult validation failed."));
			return;
		}
		UE_LOG(TABA_Logger, Warning, TEXT("%s by %s: Road request sent, hit object at (%s): %s."), *GetActorLabel(), *Requestor.ToString(), *AtGrid.ToString(), *HitResult.Component->GetName())
		UInstancedStaticMeshComponent* HitParent = Cast<UInstancedStaticMeshComponent>(HitResult.Component);
		ETileType HitTile;
		EForwardDirection HitTileDirection;
		GetTileTypeAndDirection(HitParent, HitResult.Item, HitTile, HitTileDirection);
		NewDir = HitTileDirection;

		
		//Is there even a mesh assigned for roads
		if(TileTypesAssigned.bStraightRoad)
		{
			if(RoadType == HitTile && ForwardDirection == HitTileDirection)
			{
				UE_LOG(TABA_Logger, Display, TEXT("%s by %s: Hit object is the same, skipping."), *GetActorLabel(), *Requestor.ToString());
			}
			//If we hit a road, merge roads
			else if(UEnum::GetValueAsName(HitTile).ToString().Contains("road"))
			{
				UE_LOG(TABA_Logger, Display, TEXT("%s by %s: Merging hit object."), *GetActorLabel(), *Requestor.ToString());

				const int32 Diff = HitTileDirection-ForwardDirection;


				//Placing straight road
				if(RoadType == RoadStraight)
				{
					//Hit straight, or split, but crossing
					if((HitTile == RoadStraight || HitTile == RoadSplit) && FMath::Abs(Diff) % 2 == 1)
					{
						NewType = RoadCrossing;
					}
					else if(HitTile == RoadCorner)
					{
						if(FMath::Abs(Diff) % 2 == 1)
						{
							NewDir = static_cast<EForwardDirection>((HitTileDirection+1) % 4);
							NewType = RoadSplit;
						}
					}
				}
				//Placing corner road
				else if(RoadType == RoadCorner)
				{
					//Hit a straight road, create split
					if(HitTile == RoadStraight)
					{
						//Set direction by hit directions
						if(FMath::Abs(Diff) == 2)
						{
							NewDir = ForwardDirection;
						}
						else if(Diff == -1 || Diff == 3)
						{
							NewDir = static_cast<EForwardDirection>((ForwardDirection + 1) % 4);
						}
					}
					else if(HitTile == RoadCorner)
					{
						if(ForwardDirection == HitTileDirection){}
						else if(FMath::Abs(Diff) == 2)
						{
							NewType = RoadCrossing;
						}
						else if(Diff == -1 || Diff == 3)
						{
							NewDir = static_cast<EForwardDirection>((HitTileDirection + 1) % 4);
						}
					}
				}
				
				//If a valid road was requested, remove hit instanced tile
				if(RoadType == RoadStraight || RoadType == RoadCorner)
				{
					HitParent->RemoveInstance(HitResult.Item);
					HitParent->MarkRenderStateDirty();
					UE_LOG(TABA_Logger, Display, TEXT("%s by %s: Removing instance from %s at index: %d."), *GetActorLabel(), *Requestor.ToString(), *HitResult.Component->GetName(), HitResult.Item);
				}
				else
				{
					UE_LOG(TABA_Logger, Warning, TEXT("%s by %s: Road request sent, but requested tile is not compatible: '%s' at coord (%d, %d)."), *GetActorLabel(), *Requestor.ToString(), *UEnum::GetValueAsName(RoadType).ToString(), AtGrid.X, AtGrid.Y);
				}
			}
			//Hit a tile, but it's not a road, can't merge
			else
			{
				UE_LOG(TABA_Logger, Warning, TEXT("%s by %s: Road request sent, but a '%s' already exists at the same coord (%d, %d)."), *GetActorLabel(), *Requestor.ToString(), *UEnum::GetValueAsName(HitTile).ToString(), AtGrid.X, AtGrid.Y);
			}
		}
		//No tile mesh assigned, spawn default base mesh
		else
		{
			SpawnTile(AtGrid, FindTileParentComponent(LoadedTileSetAsset->BaseTileMeshPtr.Get()));
		}
	}
	


	UStaticMesh* FoundMesh = GetMeshFromTileType(NewType);
	if(!IsValid(FoundMesh))
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s by %s: Road request sent, but couldn't find Mesh based on %s."), *GetActorLabel(), *Requestor.ToString(), *UEnum::GetValueAsName(NewType).ToString());
		return;
	}
	UInstancedStaticMeshComponent* FoundParent = FindTileParentComponent(FoundMesh, false, NewType);
	if(!IsValid(FoundParent))
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s by %s: Road request sent, but couldn't find parent ISMC for %s."), *GetActorLabel(), *Requestor.ToString(), *UEnum::GetValueAsName(NewType).ToString());
		return;
	}

	SpawnTile(AtGrid, FoundParent, NewDir);
	 

}

bool ALevelMaster::RequestRandomBool_I_Implementation(const double Weight)
{
	return InitialSeed.GetFraction()<Weight;
}

int32 ALevelMaster::RequestRandomRangeInt_I_Implementation(const int32 Max, const int32 Min)
{
	return InitialSeed.RandRange(Min, Max-1);
}

double ALevelMaster::RequestRandomFloatRange_I_Implementation(const FDoubleRange Range)
{
	double RandDouble = Range.GetUpperBoundValue()-Range.GetLowerBoundValue();
	RandDouble *= InitialSeed.GetFraction();
	RandDouble += Range.GetLowerBoundValue();
	return RandDouble;
}

void ALevelMaster::OnMeshOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(Cast<IEnemyInterface>(OtherActor))
	{
		EndMatch(false);
	}
}

void ALevelMaster::EnemyDied_I_Implementation()
{
	TotalEnemiesLeft--;
	CurrentWaveEnemiesLeft[CurrentWave]--;
	
}



