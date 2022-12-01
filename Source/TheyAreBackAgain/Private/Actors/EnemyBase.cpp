// Copyright Ónodi Kristóf. All Rights Reserved.


#include "EnemyBase.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Core/GS_TABA.h"
#include "Interfaces/LevelManagerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Libraries/GridHelperLibrary.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"


AEnemyBase::AEnemyBase()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("root"));
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("EnemyMesh");
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName("OverlapAllDynamic");
	MeshComponent->SetGenerateOverlapEvents(true);
	PrimaryActorTick.bCanEverTick = true;
	
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemyBase::SetSelfHiddenDelayed);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(!bIsActivated)
	{
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
		return;
	}

	if(IsArrivedToTarget())
	{
		GetNextTargetPosition();
	}

	BasicMoveTo(DeltaTime);
}

void AEnemyBase::SetSelfHiddenDelayed()
{
	if(!IsHidden() && !bIsActivated)
	{
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
	}
}

void AEnemyBase::EnableEnemy()
{
	bIsActivated = true;
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
}

void AEnemyBase::BasicMoveTo(const double DeltaTime)
{
	FVector Direction = CurrentTargetPosition - GetActorLocation();
	Direction.Z = 0;
	Direction.Normalize(1E-01);
	Direction = Direction * Speed * SpeedModifier * DeltaTime;
	SetActorLocation(GetActorLocation() + Direction);
}

FVector AEnemyBase::GetNextTargetPosition(const bool bInitial)
{
	if(!bInitial) CurrentTargetIndex++;
	if(CurrentTargetIndex >= LaneWaypoints.Num())
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Out of waypoints, don't know whaddoodoo. Returning to pool."), *GetActorLabel());
		SetSelfHiddenDelayed();
		ReturnToPool();
		return FVector();
	}

	CurrentTargetPosition = UGridHelperLibrary::GridPositionToWorld(LaneWaypoints[CurrentTargetIndex], this) + HeightOffset;
	this->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTargetPosition));
	return CurrentTargetPosition;
}

bool AEnemyBase::IsArrivedToTarget() const
{
	FVector Distance = CurrentTargetPosition - GetActorLocation();
	Distance.Z = 0;
	if(Distance.Length() < ArriveThreshold)
	{
		return true;
	}
	return false;
}

void AEnemyBase::RegisterPool_I_Implementation(UActorComponent* PoolManager)
{
	IPooledItemInterface::RegisterPool_I_Implementation(PoolManager);

	if(!IsValid(PoolManagerComponent))PoolManagerComponent=Cast<UObjectPoolManagerComponent>(PoolManager);
	PoolManagerComponent->OnPoolWiped.AddUniqueDynamic(this, &AEnemyBase::DestroySelf);

}

void AEnemyBase::ActivatePoolItem_I_Implementation()
{
	
}

void AEnemyBase::DestroySelf()
{
	this->Destroy();
}

void AEnemyBase::ReturnToPool()
{
	if(!IsValid(PoolManagerComponent))
	{
		UE_LOG(TABA_Logger, Error, TEXT("%s: Enemy was not spawned by poolmanager or IPooledItemInterface::RegisterPool_I() is not implemented."), *GetActorLabel());
		return;
	}
	PoolManagerComponent->ReturnItemToPool(this);
}

void AEnemyBase::InitEnemy_I_Implementation(const UEnemyBaseDA* EnemyData)
{
	IEnemyInterface::InitEnemy_I_Implementation(EnemyData);
	
	if(!IsValid(EnemyData))
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: Enemy initialization failed. Couln't get the data asset"), *GetActorLabel());
		ReturnToPool();
		return;
	}

	EnemyName = EnemyData->EnemyName;
	IsFlying = EnemyData->IsFlying;
	MaxHealth = EnemyData->MaxHealth;
	CurrentHealth = MaxHealth;
	Speed = EnemyData->Speed;
	SpeedModifier = 1.0;
	
	if(!EnemyData->MeshPtr.IsValid())
	{
		UE_LOG(TABA_Logger, Display, TEXT("%s: Enemy initialization failed. Couln't get the mesh from data asset - %s"), *GetActorLabel(), *EnemyData->GetName());
		ReturnToPool();
		return;
	}
	MeshComponent->SetStaticMesh(EnemyData->MeshPtr.Get());
	
}

void AEnemyBase::ActivateEnemy_I_Implementation(const TArray<FGrid2D>& Waypoints)
{
	IEnemyInterface::ActivateEnemy_I_Implementation(Waypoints);
	CurrentTargetIndex = 0;
	LaneWaypoints = Waypoints;
	HeightOffset.Z = GetActorLocation().Z;
	GetNextTargetPosition();
	EnableEnemy();	
}

void AEnemyBase::TakeDamage_I_Implementation(const double Damage)
{

	IEnemyInterface::TakeDamage_I_Implementation(Damage);

	CurrentHealth -= Damage;
	if(CurrentHealth <= 0)
	{
		OnEnemyDied();
	}
}

bool AEnemyBase::IsDead_Implementation()
{
	return !bIsActivated;
}

void AEnemyBase::OnEnemyDied()
{
	if(bIsActivated)
	{
		bIsActivated = false;
		
		Cast<AGS_TABA>(UGameplayStatics::GetGameState(this))->OnEnemyDied_Implementation(IsFlying, GetActorTransform());

		
		SetSelfHiddenDelayed();
		ReturnToPool();
	}
}

