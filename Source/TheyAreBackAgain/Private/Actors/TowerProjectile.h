// Copyright Ónodi Kristóf. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PooledItemInterface.h"
#include "Interfaces/ProjectileInterface.h"
#include "TowerProjectile.generated.h"

UCLASS(BlueprintType)
class THEYAREBACKAGAIN_API ATowerProjectile : public AActor, public IProjectileInterface, public IPooledItemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATowerProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AActor* TrackedTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Speed = 200;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double ProjectileDamage = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UObjectPoolManagerComponent* PoolManagerComponent;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void ActivateProjectile_I_Implementation(AActor* Target, double Damage) override;

	virtual void RegisterPool_I_Implementation(UActorComponent* PoolManager) override;

	UFUNCTION()
	void DestroySelf();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SpawnVFX(FTransform Transform);
};
