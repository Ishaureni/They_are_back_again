// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/EnemyInterface.h"
#include "Interfaces/PooledItemInterface.h"
#include "EnemyBase.generated.h"

UCLASS()
class THEYAREBACKAGAIN_API AEnemyBase : public AActor, public IPooledItemInterface, public IEnemyInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyBase();
public:
	/** Display name */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName EnemyName = "NewEnemy";

	/** Set to true if the enemy can fly
	 *  If false only ground lane can spawn them
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool IsFlying = true;

	/** Initial health pool */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double MaxHealth = 100.0;

	/** Base speed without modifiers */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double Speed = 5.0;

	/** Mesh component */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	/** True if enemy is active */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	bool bIsActivated = false;
	
	/** Initial health pool */
	UPROPERTY(Transient, BlueprintReadOnly, EditAnywhere)
	double CurrentHealth;

	/** Base speed without modifiers */
	UPROPERTY(Transient, BlueprintReadOnly, EditAnywhere)
	double SpeedModifier;

	/**Waypoints to follow*/
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	TArray<FGrid2D> LaneWaypoints;
	
	/** Waypoint index currently used */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	int32 CurrentTargetIndex;
	
	/** Waypoint heading to currently in world */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	FVector CurrentTargetPosition;
	
	/** Waypoints to follow */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double ArriveThreshold = 10;
	
	/** Height offset */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	FVector HeightOffset;
	
	/** Pool manager component */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	UObjectPoolManagerComponent* PoolManagerComponent;


	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetSelfHiddenDelayed();

	void EnableEnemy();

	void BasicMoveTo(const double Deltatime);

	FVector GetNextTargetPosition(bool bInitial = false);

	bool IsArrivedToTarget() const;

	virtual void RegisterPool_I_Implementation(UActorComponent* PoolManager) override;
	
	virtual void ActivatePoolItem_I_Implementation() override;

	UFUNCTION()
	void DestroySelf();
	
	void ReturnToPool();

	virtual void InitEnemy_I_Implementation(const UEnemyBaseDA* EnemyData) override;

	virtual void ActivateEnemy_I_Implementation(const TArray<FGrid2D>& Waypoints) override;

	void TakeDamage_I_Implementation(const double Damage) override;

	virtual bool IsDead_Implementation() override;
	
	void OnEnemyDied();
};




