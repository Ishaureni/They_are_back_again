// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolManagerComponent.h"
#include "TowerProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PooledItemInterface.h"
#include "TowerBase.generated.h"

UCLASS()
class THEYAREBACKAGAIN_API ATowerBase : public AActor, public IPooledItemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATowerBase();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UClass> ProjectileClass; 

	UPROPERTY(EditDefaultsOnly)
	double Damage = 40;

	UPROPERTY(EditDefaultsOnly)
	double Range = 500;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* RangeCollider;

	/**Given in seconds*/
	UPROPERTY(EditDefaultsOnly)
	double FireRate = 2.0;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly)
	FName Name;

	UPROPERTY(EditDefaultsOnly)
	FName Description;
	
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> EnemiesInRange;
	
	UPROPERTY(VisibleAnywhere)
	AActor* Closest;
	
	UPROPERTY(VisibleAnywhere)
	UObjectPoolManagerComponent* PoolManagerComponent;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Tower;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Turret;

	UPROPERTY(EditAnywhere)
	USceneComponent* TurretEnd;

	UPROPERTY()
	FTimerHandle ShootTimer;

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;

	void EnableTower();
	
	UFUNCTION()
	void OnMeshOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnMeshOverlapEnded(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	AActor* GetClosestEnemy();

	UFUNCTION()
	void TryShootOnClosestEnemy();

	virtual void ActivatePoolItem_I_Implementation() override;

	virtual void RegisterPool_I_Implementation(UActorComponent* PoolManager) override;

	UFUNCTION()
	void DestroySelf(bool LevelDefeated);
};





