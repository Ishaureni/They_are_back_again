// Copyright Ónodi Kristóf. All Rights Reserved.


#include "EnemySpawner.h"

#include "AudioMixerBlueprintLibrary.h"
#include "DataObjects/Enums.h"
#include "Libraries/GridHelperLibrary.h"


AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("root"));
	GroundSpawnPosition = CreateDefaultSubobject<USceneComponent>("GroundSpawner");
	GroundSpawnPosition -> SetupAttachment(GetRootComponent());
	AirSpawnPosition = CreateDefaultSubobject<USceneComponent>("AirSpawner");
	AirSpawnPosition -> SetupAttachment(GetRootComponent());

}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	GroundSpawnPosition -> SetRelativeLocation(FVector(0,0,UGridHelperLibrary::GetGridGlobalScale(this)*20));
	AirSpawnPosition -> SetRelativeLocation(FVector(0,0,UGridHelperLibrary::GetGridGlobalScale(this)*120));

}

void AEnemySpawner::ActivateSpawner()
{
	UE_LOG(TABA_Logger, Display, TEXT("%s: Spawner initialization done, activated."), *GetActorLabel());
	LevelMaster->OnNextWaveStarted.AddUniqueDynamic(this, &AEnemySpawner::StartNextWave);
	LevelMaster->OnMatchEnded.AddUniqueDynamic(this, &AEnemySpawner::DeActivateSpawner);
	IsActive = true;
}

void AEnemySpawner::DeActivateSpawner(bool bLevelDefeated)
{
	UE_LOG(TABA_Logger, Display, TEXT("%s: Spawner deactivated."), *GetActorLabel());
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	LevelMaster->OnMatchEnded.RemoveDynamic(this, &AEnemySpawner::DeActivateSpawner);
	LevelMaster->OnNextWaveStarted.RemoveDynamic(this, &AEnemySpawner::StartNextWave);
	IsActive = false;
}

void AEnemySpawner::StartNextWave(const int32 WaveIndex)
{
	if(!IsActive && WaveIndex == 0) ActivateSpawner();
	if(!IsActive) return;
	UE_LOG(TABA_Logger, Display, TEXT("%s: spawner starting wave index %d."), *GetActorLabel(), WaveIndex);
	
	CurrentWaveID = WaveIndex;
	FEnemyWave CurrentWave;
	if(!LevelMaster->GetWave(LaneID, CurrentWaveID, CurrentWave))
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Spawner has no waves to spawn, deactivating it."), *GetActorLabel());
		DeActivateSpawner();
		return;
	}
	

	LoadedWaveAirEnemies.Empty();
	LoadedWaveGroundEnemies.Empty();
	for(UEnemyBaseDA* e:CurrentWave.AdditionalEnemies)
	{
		if(e->IsFlying) LoadedWaveAirEnemies.Add(e);
		else LoadedWaveGroundEnemies.Add(e);
	}
	
	const double SpawnRate = Execute_RequestRandomFloatRange_I(LevelMaster, CurrentWave.SpawnInterval);
	const double InitialDelay = CurrentWave.InitialDelay;
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, SpawnRate, true, InitialDelay);

}


void AEnemySpawner::InitSpawnerActor_I_Implementation(FTransform& Transform, const int32 LaneIndex, const FGrid2D TargetCoord, const FEnemyLane& LaneInfo)
{
	ISpawnerInterface::InitSpawnerActor_I_Implementation(Transform, LaneIndex, TargetCoord, LaneInfo);
	UE_LOG(TABA_Logger, Display, TEXT("%s: spawner initialization started."), *GetActorLabel());

	CurrentWaveSpawnLeft.Empty();
	TotalSpawnedAmount =0;
	LaneID = LaneIndex;
	bIsGroundLane = LaneInfo.bIsGroundLane;

	ValidateWaypoints(TargetCoord, LaneInfo, LaneIndex);

	Waypoints.Add(TargetCoord);
	
	if(bIsGroundLane)
	{
		SpawnRoad(TargetCoord, LaneIndex);
	}

	for(int32 i = 0; i < LaneInfo.Waves.Num(); ++i)
	{
		CurrentWaveSpawnLeft.Add(LaneInfo.Waves[i].EnemyAmount);
	}
	
	FRotator Rotator = FRotator(0,0,0);
	const FGrid2D DirUnit = bIsGroundLane?GroundWaypoints[1].Subtract(GroundWaypoints[0]).Unit():Waypoints[1].Subtract(Waypoints[0]).Unit();
	if(DirUnit.X == -1) Rotator.Yaw = 180;
	else if(DirUnit.Y == 1) Rotator.Yaw = 90;
	else if(DirUnit.Y == -1) Rotator.Yaw = 270;

	Transform.SetRotation(Rotator.Quaternion());

	LevelMaster = Cast<ALevelMaster>(Owner);

	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemySpawner::ActivateSpawner);

}

void AEnemySpawner::ActivatePoolItem_I_Implementation()
{
	IPooledItemInterface::ActivatePoolItem_I_Implementation();


}

void AEnemySpawner::RegisterPool_I_Implementation(UActorComponent* PoolManager)
{
	IPooledItemInterface::RegisterPool_I_Implementation(PoolManager);

	ObjectPoolManager = Cast<UObjectPoolManagerComponent>(PoolManager);

	ObjectPoolManager->OnPoolWiped.AddUniqueDynamic(this, &AEnemySpawner::DestroySelf);
}

bool AEnemySpawner::TryCalculateStraightenerCoord(FGrid2D& OutCoord, const FGrid2D From, const FGrid2D To, const int32 LaneIndex, const TArray<FGrid2D>& OriginalWaypoints) const
{
	const FGrid2D MixA = From.Mix(To);
	if(FHitResult Hit; MixA.IsContained(OriginalWaypoints) || MixA.IsContained(GroundWaypoints) || UGridHelperLibrary::IsGridOccupied(Owner,MixA, Hit))
	{
		const FGrid2D MixB = From.Mix(To, true);
		if(MixB.IsContained(OriginalWaypoints) || MixB.IsContained(GroundWaypoints))
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: GroundWaypoint couldn't be generated in lane #%d after coord: %d, %d. Tried creating %d, %d and %d, %d but they're conflicting with already set waypoints."), *GetActorLabel(), LaneIndex, From.X, From.Y, MixA.X, MixA.Y, MixB.X, MixB.Y);
			return false;
		}
		OutCoord = MixB;
		return true;
	}
	OutCoord = MixA;
	return true;
}

void AEnemySpawner::ValidateWaypoints(const FGrid2D TargetCoord, const FEnemyLane& LaneInfo, const int32 LaneIndex)
{
	Waypoints.Empty();
	GroundWaypoints.Empty();
	Waypoints.Add(LaneInfo.SpawnerTile);
	if(bIsGroundLane)
	{
		GroundWaypoints.Add(LaneInfo.SpawnerTile);
		FGrid2D NewCoord = LaneInfo.Waypoints.Num()>0 ? LaneInfo.Waypoints[0] : TargetCoord;
		if(!LaneInfo.SpawnerTile.IsStraight(NewCoord))
		{
			if(TryCalculateStraightenerCoord(NewCoord, LaneInfo.SpawnerTile, NewCoord, LaneIndex, LaneInfo.Waypoints)) GroundWaypoints.Add(NewCoord);
		}
	}
	
	for(int32 i = 0; i < LaneInfo.Waypoints.Num(); ++i)
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Validating waypoint: %s."), *GetActorLabel(), *LaneInfo.Waypoints[i].ToString());

		if(LaneInfo.Waypoints[i].IsContained(Waypoints))
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: Discarding duplicate waypoint found in lane %d at coord: %s"), *GetActorLabel(), LaneIndex, *LaneInfo.Waypoints[i].ToString());
		}
		else if(LaneInfo.Waypoints[i].Equals(TargetCoord))
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: Discarding waypoint found in lane %d at coord: %s. It equals the target tile's coord, gets added automatically as last waypoint."),	*GetActorLabel(), LaneIndex, *LaneInfo.Waypoints[i].ToString());
		}
		else
		{
			Waypoints.Add(LaneInfo.Waypoints[i]);
			
			if(bIsGroundLane)
			{
				GroundWaypoints.Add(LaneInfo.Waypoints[i]);

				FGrid2D NextCoord = i < LaneInfo.Waypoints.Num()-1 ? LaneInfo.Waypoints[i+1] : TargetCoord;
				if(!LaneInfo.Waypoints[i].IsStraight(NextCoord))
				{
					FGrid2D NewCoord;
					if(TryCalculateStraightenerCoord(NewCoord, LaneInfo.Waypoints[i], NextCoord, LaneIndex, LaneInfo.Waypoints))
					{
						UE_LOG(TABA_Logger, Display, TEXT("%s: A new ground waypoint in lane #%d was inserted: %s."),	*GetActorLabel(), LaneIndex, *NewCoord.ToString());
						GroundWaypoints.Add(NewCoord);
					}
				}
			}
		}
	}
}

void AEnemySpawner::SpawnRoad(const FGrid2D TargetCoord, const int32 LaneIndex)
{
	GroundWaypoints.Add(TargetCoord);

	EForwardDirection LastDirection = Up;
	for(int32 Path = 0; Path < GroundWaypoints.Num() - 1; ++Path)
	{
			
		FGrid2D From = GroundWaypoints[Path];
		FGrid2D To = GroundWaypoints[Path+1];
		FGrid2D DirUnit = To.Subtract(From);
		if(DirUnit.X != 0 && DirUnit.Y != 0)
		{
			UE_LOG(TABA_Logger, Warning, TEXT("%s: Road generation will fail, coords couldn't be aligned in lane %d at %d, %d."),	*GetActorLabel(), LaneIndex, GroundWaypoints[Path].X, GroundWaypoints[Path].Y);
		}
		DirUnit = DirUnit.Unit();
		EForwardDirection ToDirection = DirUnit.GetForwardDirection();
		ETileType NextTile = RoadStraight;
			
		for(int32 PathStep = 0; PathStep < From.LinearDistance(To); ++PathStep)
		{
			if(Path == 0 && PathStep == 0) continue;
				
			if(Cast<ISpawnerInterface>(Owner))
			{
				//Is next tile straight or corner
				//First part can only be straight
				if(Path > 0)
				{
					//first tile in part can be corner
					if(PathStep == 0)
					{
						NextTile = FMath::Abs(ToDirection-LastDirection)%2 == 1 ? RoadCorner : RoadStraight;
					}
					else if(PathStep == 1) NextTile = RoadStraight;
				}

				const FGrid2D CurrentGrid = From.AddScaled(DirUnit, PathStep);

				Execute_RequestRoad_I(Owner, CurrentGrid, NextTile, NextTile==RoadCorner?UGridHelperLibrary::GetCornerForwardDirection(LastDirection, ToDirection):ToDirection, GetFName());

			}
		}
		LastDirection = ToDirection;
	}
}

UEnemyBaseDA* AEnemySpawner::GetRandomEnemy(const TArray<UEnemyBaseDA*>& EnemyArray) const
{
	return EnemyArray[ Execute_RequestRandomRangeInt_I(LevelMaster, EnemyArray.Num(),0) ];
}

void AEnemySpawner::SpawnEnemy()
{
	if(CurrentWaveSpawnLeft[CurrentWaveID] <= 0) return;
	if(Cast<ISpawnerInterface>(Owner))
	{
		bool bRequestLandUnit = bIsGroundLane && Execute_RequestRandomBool_I(Owner, 0.5);
		const bool bHasGlobalPool = LevelMaster->GetLoadedEnemies(bRequestLandUnit).Num() > 0;
		const bool bHasLanePool = bRequestLandUnit ? LoadedGroundEnemies.Num() > 0 : LoadedAirEnemies.Num() > 0;
		const bool bHasWavePool = bRequestLandUnit ? LoadedWaveGroundEnemies.Num() > 0 : LoadedWaveAirEnemies.Num() > 0;
		UEnemyBaseDA* NewEnemyType;

		if(bHasGlobalPool && Execute_RequestRandomBool_I(Owner, 0.4))
		{
			NewEnemyType = GetRandomEnemy(LevelMaster->GetLoadedEnemies(bRequestLandUnit));
		}
		else if(bHasLanePool && Execute_RequestRandomBool_I(Owner, 0.4))
		{
			NewEnemyType = GetRandomEnemy(bRequestLandUnit ? LoadedGroundEnemies : LoadedAirEnemies);
		}
		else if(bHasWavePool)
		{
			NewEnemyType = GetRandomEnemy(bRequestLandUnit ? LoadedWaveGroundEnemies : LoadedWaveAirEnemies);
		}
		else
		{
			TArray<UEnemyBaseDA*> TempArray = LevelMaster->GetLoadedEnemies(bRequestLandUnit);
			TempArray.Append(bRequestLandUnit ? LoadedGroundEnemies : LoadedAirEnemies);
			if(TempArray.Num()==0 && bRequestLandUnit)
			{
				bRequestLandUnit = false;
				TempArray = LevelMaster->GetLoadedEnemies(false);
				TempArray.Append(LoadedAirEnemies);
				TempArray.Append(LoadedWaveAirEnemies);
				if(TempArray.Num() == 0)
				{
					UE_LOG(TABA_Logger, Error, TEXT("%s: Can't spawn a single unit."), *GetActorLabel());
					return;
				}
			}
			NewEnemyType = GetRandomEnemy(TempArray);
		}

		if(!IsValid(NewEnemyType))
		{
			UE_LOG(TABA_Logger, Error, TEXT("%s: Couldn't spawn a unit."), *GetActorLabel());
			return;			
		}
		
		AActor* NewEnemy = ObjectPoolManager->RequestItemFromPool(AEnemyBase::StaticClass(), bRequestLandUnit ? GroundSpawnPosition->GetComponentTransform() : AirSpawnPosition->GetComponentTransform());
		CurrentWaveSpawnLeft[CurrentWaveID]--;
		TotalSpawnedAmount++;

		IEnemyInterface::Execute_InitEnemy_I(NewEnemy, NewEnemyType);
		IEnemyInterface::Execute_ActivateEnemy_I(NewEnemy, bRequestLandUnit ? GroundWaypoints : Waypoints);
	}
	else
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: Owner has no ISpawnerInterface implemented, or Owner is not a level master. Owner: %s"), *GetActorLabel(), Owner? *Owner.GetName() : TEXT("NONE"));
	}
}

void AEnemySpawner::DestroySelf()
{
	DeActivateSpawner();
	ObjectPoolManager->ReturnItemToPool(this);
	//this->Destroy();
}




