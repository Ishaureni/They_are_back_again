// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataObjects/Structures.h"
#include "Interfaces/AudioInterface.h"
#include "ObjectPoolManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWipePool);

UCLASS(Blueprintable, ClassGroup=("TABA"), meta=(BlueprintSpawnableComponent))
class THEYAREBACKAGAIN_API UObjectPoolManagerComponent : public UActorComponent, public IAudioInterface
{
	GENERATED_BODY()

public:
	UObjectPoolManagerComponent();

	UPROPERTY(BlueprintAssignable)
	FWipePool OnPoolWiped;

	/**Audio component*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* AudioComponent;
	
	/**Unused pooled items are placed matched with this actor's transform*/
	UPROPERTY(EditAnywhere)
	AActor* PoolParent;

	/**Available pools to get items from*/
	UPROPERTY(VisibleAnywhere)
	TArray<FObjectPool> AvailablePools;

	/**Active items requested from pools*/
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> ActiveItems;
	
	/**Active items requested from pools*/
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> ActiveEnemies;
	
	/**Exceding amount will be trimmed from pools when clear runs*/
	UPROPERTY(EditAnywhere)
	int32 PoolReserveAmount = 5;

	/**Run clearing of pools at a given rate
	 * @param ClearRunInterval interval in seconds
	 */
	UPROPERTY(EditAnywhere)
	double ClearRunInterval = 30;
	
	/**Spawn params for setting collision handling*/
	FActorSpawnParameters SpawnParameters;

	UPROPERTY()
	FTimerHandle ClearPoolsTimerHandle;

	/**
	 *@param bLogEnabled if set to true pool manager logs activities
	 */
	UPROPERTY(EditAnywhere)
	bool bLogEnabled = false;
	
protected:
	virtual void BeginPlay() override;

public:


	/**Gets the index of pool with 'ItemClass'
	 * @param ItemClass Class of pool to look for
	 * @return Returns found or created pool index
	 */
	int32 FindOrCreatePool(TSubclassOf<AActor> ItemClass);

	/**Creates new pool and returns index
	 * @param ItemClass Assigned class of the new pool
	 * @return Returns the created pool index in available pools array
	 */
	int32 CreatePool(TSubclassOf<AActor> ItemClass);

	/**Returns item of class from pool, and sets transform, removes from available pool
	 *@param ItemClass Class of item to return
	 *@param Transform Sets item transform
	 *@param bActivateWithInterface set this true for an automatic activation
	 */
	UFUNCTION(BlueprintCallable)
	AActor* RequestItemFromPool(TSubclassOf<AActor> ItemClass, const FTransform Transform, const bool bActivateWithInterface = false);

	/**Creates a new item in pool.
	 * @param ItemClass class of item to create
	 * @param bAddToPool if true, adds item to pool and deactivates it for later use.
	 * @return Returns the actor
	 */
	AActor* CreateNewPoolItem(TSubclassOf<AActor> ItemClass, const bool bAddToPool = false);

	/**Sets item visibility and tick
	 * @param Item Affected item
	 * @param bEnableTick if true sets item's tick to enabled
	 */
	static void EnableItem(AActor* Item, bool bEnableTick = false);
	
	/**Sets item hidden and tick disabled
	 * @param Item Affected item
	 */
	void DisableItem(AActor* Item) const;

	/**Returns item to it's pool and calls disable on it.*/
	void ReturnItemToPool(AActor* Item);

	/**Preloads items for later use
	 * @param ItemClass Actor subclass to preload
	 * @param Amount Determines how many actors will be created
	 * @return Return an AActor pointer array of the created items
	 */
	TArray<AActor*> PreloadItemsInPool(const TSubclassOf<AActor> ItemClass, const int32 Amount);
	
	/**Pops and destroys items over reserve, with dynamic adjust
	 * @param bDoCompleteWipe if true destroys all active and reserve items
	 */
	void ClearPools(const bool bDoCompleteWipe = false);

	void ClearPoolsTimed();

	void PlayShootAt(FVector Location);
	
	UFUNCTION(BlueprintCallable)
	void WipePool();

	UFUNCTION(BlueprintCallable)
	virtual void PlayShoot_Implementation() override;
};


