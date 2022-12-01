// Copyright Ónodi Kristóf. All Rights Reserved..


#include "TowerProjectile.h"

#include "Interfaces/EnemyInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"


ATowerProjectile::ATowerProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorTickEnabled(false);

}

void ATowerProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATowerProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(!IsValid(TrackedTarget) || !Cast<IEnemyInterface>(TrackedTarget))
	{
		SetActorTickEnabled(false);
		SetActorHiddenInGame(true);
		PoolManagerComponent->ReturnItemToPool(this);
	}
	else if(IEnemyInterface::Execute_IsDead(TrackedTarget) || TrackedTarget->IsHidden() || TrackedTarget->GetActorLocation().Z < -10)
	{
		TrackedTarget = nullptr;
		SetActorTickEnabled(false);
		SetActorHiddenInGame(true);
		PoolManagerComponent->ReturnItemToPool(this);
	}
	else if(IsValid(TrackedTarget))
	{
		FVector TargetPoint = TrackedTarget->GetActorLocation() + FVector().UpVector*20;
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPoint));

		SetActorLocation(GetActorLocation()+GetActorForwardVector()*Speed*DeltaTime);

		double CurrentDistance = (GetActorLocation()-TargetPoint).Length();

		if(CurrentDistance < 20)
		{
			SpawnVFX(GetActorTransform());
			IEnemyInterface::Execute_TakeDamage_I(TrackedTarget, ProjectileDamage);
			SetActorHiddenInGame(true);
			SetActorTickEnabled(false);

			PoolManagerComponent->ReturnItemToPool(this);
		}
	}

}

void ATowerProjectile::ActivateProjectile_I_Implementation(AActor* Target, double Damage)
{
	IProjectileInterface::ActivateProjectile_I_Implementation(Target, Damage);
	TrackedTarget = Target;
	ProjectileDamage = Damage;
	SetActorTickEnabled(true);
}

void ATowerProjectile::RegisterPool_I_Implementation(UActorComponent* PoolManager)
{
	IPooledItemInterface::RegisterPool_I_Implementation(PoolManager);
	PoolManagerComponent = Cast<UObjectPoolManagerComponent>(PoolManager);
	PoolManagerComponent->OnPoolWiped.AddUniqueDynamic(this, &ATowerProjectile::DestroySelf);
}

void ATowerProjectile::DestroySelf()
{
	this->Destroy();
}

void ATowerProjectile::SpawnVFX_Implementation(FTransform Transform)
{
}





