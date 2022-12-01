// Copyright Ónodi Kristóf. All Rights Reserved..


#include "TowerBase.h"

#include "LevelMaster.h"
#include "TowerProjectile.h"
#include "Interfaces/EnemyInterface.h"
#include "Interfaces/ProjectileInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("root"));
	Tower = CreateDefaultSubobject<UStaticMeshComponent>("Tower");
	Tower->SetupAttachment(GetRootComponent());
	Turret = CreateDefaultSubobject<UStaticMeshComponent>("Turret");
	Turret->SetupAttachment(Tower);
	TurretEnd = CreateDefaultSubobject<USceneComponent>("TurretEnd");
	TurretEnd->SetupAttachment(Turret);
	RangeCollider = CreateDefaultSubobject<USphereComponent>("RangeCollider");
	RangeCollider->SetupAttachment(GetRootComponent());
	RangeCollider->SetGenerateOverlapEvents(true);
	RangeCollider->SetCollisionProfileName("OverlapAllDynamic");
}

void ATowerBase::BeginPlay()
{
	Super::BeginPlay();

}

void ATowerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(IsValid(Closest) && Closest->GetActorLocation().Z>-10)
	{
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(Turret->GetComponentLocation(), Closest->GetActorLocation());
		
		Turret->SetWorldRotation(LookAtRot);
	}
	
}

void ATowerBase::EnableTower()
{
	SetActorTickEnabled(true);
	RangeCollider->OnComponentBeginOverlap.AddDynamic(this, &ATowerBase::OnMeshOverlapped);
	RangeCollider->OnComponentEndOverlap.AddDynamic(this, &ATowerBase::OnMeshOverlapEnded);
	Cast<ALevelMaster>(PoolManagerComponent->GetOwner())->OnMatchEnded.AddUniqueDynamic(this, &ATowerBase::DestroySelf);
	RangeCollider->SetSphereRadius(Range);
	GetWorldTimerManager().SetTimer(ShootTimer, this, &ATowerBase::TryShootOnClosestEnemy, FireRate, true);
	UE_LOG(TABA_Logger, Display, TEXT("%s: Tower enabled"), *GetActorLabel());
}

void ATowerBase::OnMeshOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(Cast<IEnemyInterface>(OtherActor))EnemiesInRange.Add(OtherActor);
}

void ATowerBase::OnMeshOverlapEnded(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(Cast<IEnemyInterface>(OtherActor))EnemiesInRange.RemoveSingle(OtherActor);
}

AActor* ATowerBase::GetClosestEnemy()
{

	Closest = EnemiesInRange[0];
	
	double Distance = (Closest->GetActorLocation()- GetActorLocation()).SquaredLength();
	for(AActor* a:EnemiesInRange)
	{
		if((GetActorLocation()- a->GetActorLocation()).SquaredLength() < Distance) Closest = a;
	}
	return Closest;
}

void ATowerBase::TryShootOnClosestEnemy()
{

	if(EnemiesInRange.Num()>0 && IsValid(EnemiesInRange[0]))
	{
		Closest = GetClosestEnemy();

		AActor* Projectile = PoolManagerComponent->RequestItemFromPool(ProjectileClass, TurretEnd->GetComponentTransform());
		if(Cast<IProjectileInterface>(Projectile))IProjectileInterface::Execute_ActivateProjectile_I(Projectile, Closest, Damage);

	}
}

void ATowerBase::ActivatePoolItem_I_Implementation()
{
	IPooledItemInterface::ActivatePoolItem_I_Implementation();
	EnableTower();
}

void ATowerBase::RegisterPool_I_Implementation(UActorComponent* PoolManager)
{
	PoolManagerComponent = Cast<UObjectPoolManagerComponent>(PoolManager);
	GetWorldTimerManager().SetTimerForNextTick(this, &ATowerBase::EnableTower);
}

void ATowerBase::DestroySelf(bool LevelDefeated)
{
	Cast<ALevelMaster>(PoolManagerComponent->GetOwner())->OnMatchEnded.RemoveDynamic(this, &ATowerBase::DestroySelf);
	RangeCollider->OnComponentBeginOverlap.RemoveDynamic(this, &ATowerBase::OnMeshOverlapped);
	RangeCollider->OnComponentEndOverlap.RemoveDynamic(this, &ATowerBase::OnMeshOverlapEnded);
	PoolManagerComponent->ReturnItemToPool(this);
}
