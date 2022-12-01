// Copyright Ónodi Kristóf. All Rights Reserved.


#include "ObjectPoolManagerComponent.h"

#include "EnemySpawner.h"
#include "Components/AudioComponent.h"
#include "Interfaces/ProjectileInterface.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"


UObjectPoolManagerComponent::UObjectPoolManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UObjectPoolManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!IsValid(PoolParent))PoolParent=this->GetOwner();
	SpawnParameters.Owner = this->GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetOwner()->GetWorldTimerManager().SetTimer(ClearPoolsTimerHandle, this, &UObjectPoolManagerComponent::ClearPoolsTimed, ClearRunInterval, true);
	if(!IsValid(AudioComponent))AudioComponent = Cast<UAudioComponent>(GetOwner()->GetComponentByClass(UAudioComponent::StaticClass()));
}


int32 UObjectPoolManagerComponent::FindOrCreatePool(const TSubclassOf<AActor> ItemClass)
{
	if(AvailablePools.Num()>0)
	{
		for(int32 i = 0; i < AvailablePools.Num(); ++i)
		{
			if(AvailablePools[i].IsClassOf(ItemClass)) return i;
		}
	}
	return CreatePool(ItemClass);
}

int32 UObjectPoolManagerComponent::CreatePool(const TSubclassOf<AActor> ItemClass)
{
	AvailablePools.Add(FObjectPool(ItemClass));
	return AvailablePools.Num()-1;
}

AActor* UObjectPoolManagerComponent::RequestItemFromPool(TSubclassOf<AActor> ItemClass, const FTransform Transform, const bool bActivateWithInterface)
{

	const int32 PoolIndex = FindOrCreatePool(ItemClass);
	AActor* ReturnItem;
	if(AvailablePools[PoolIndex].HasAvailableItem())
	{
		ReturnItem = AvailablePools[PoolIndex].AvailablePool.Pop();
	}
	else
	{
		ReturnItem = CreateNewPoolItem(ItemClass);
		if(bLogEnabled) UE_LOG(TABA_Logger, Log, TEXT("%s: %s created"), *GetOwner()->GetActorLabel(), *ReturnItem->GetName());
	}
	ReturnItem->SetActorTransform(Transform);

	if(bActivateWithInterface)
	{
		if(Cast<IPooledItemInterface>(ReturnItem)) IPooledItemInterface::Execute_ActivatePoolItem_I(ReturnItem);
		else if(bLogEnabled) UE_LOG(TABA_Logger, Warning, TEXT("%s: %s not implements pooled item interface"), *GetOwner()->GetActorLabel(), *ReturnItem->GetName());
	}

	if(Cast<IEnemyInterface>(ReturnItem)) ActiveEnemies.Add(ReturnItem);
	else ActiveItems.Add(ReturnItem);
	if(Cast<IProjectileInterface>(ReturnItem)) PlayShootAt(Transform.GetTranslation());
	return ReturnItem;
}

AActor* UObjectPoolManagerComponent::CreateNewPoolItem(const TSubclassOf<AActor> ItemClass, const bool bAddToPool)
{
	const int32 PoolIndex = FindOrCreatePool(ItemClass);
	
	AActor* NewItem= GetWorld()->SpawnActor(ItemClass, 0, 0, SpawnParameters);
		NewItem->AttachToActor(PoolParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if(Cast<IPooledItemInterface>(NewItem)) IPooledItemInterface::Execute_RegisterPool_I(NewItem, this);

	if(bLogEnabled) UE_LOG(TABA_Logger, Log, TEXT("%s: %s created"), *GetOwner()->GetActorLabel(), *NewItem->GetName());

	if(bAddToPool)
	{
		AvailablePools[PoolIndex].AddItem(NewItem);
		DisableItem(NewItem);
	}
	
	return NewItem;
}

void UObjectPoolManagerComponent::EnableItem(AActor* Item, const bool bEnableTick)
{
	Item->SetActorHiddenInGame(true);
	Item->SetActorTickEnabled(bEnableTick);
}

void UObjectPoolManagerComponent::DisableItem(AActor* Item) const
{
	Item->SetActorHiddenInGame(false);
	Item->SetActorTickEnabled(false);
	Item->SetActorLocation(FVector(0,0,-1000));
}

void UObjectPoolManagerComponent::ReturnItemToPool(AActor* Item)
{
	const int32 PoolIndex = FindOrCreatePool(Item->GetClass());
	ActiveItems.RemoveSingle(Item);
	if(ActiveEnemies.RemoveSingle(Item)==1)
	{
		ISpawnerInterface::Execute_EnemyDied_I(PoolParent);
	}
	AvailablePools[PoolIndex].AddItem(Item);
	DisableItem(Item);
}

TArray<AActor*> UObjectPoolManagerComponent::PreloadItemsInPool(const TSubclassOf<AActor> ItemClass, const int32 Amount)
{
	if(Amount<0) return TArray<AActor*>();
	TArray<AActor*> PreloadedActors;
	for(int32 i = 0; i < Amount; ++i)
	{
		PreloadedActors.Add(CreateNewPoolItem(ItemClass, true));
	}
	return PreloadedActors;
}

//NOTE: If too many pools and items will be used, then should spread across a few frames
void UObjectPoolManagerComponent::ClearPools(const bool bDoCompleteWipe)
{
	if(bDoCompleteWipe)
	{
		OnPoolWiped.Broadcast();
	}
	for(int32 i = 0; i < AvailablePools.Num(); ++i)
	{
		int32 j = AvailablePools[i].AvailablePool.Num();
		j -= AvailablePools[i].ExtraReserve;
		j -= PoolReserveAmount;
		AvailablePools[i].CalculateReserve(j);
		while(--j >= 0)
		{
			AvailablePools[i].AvailablePool.Pop()->Destroy();
		}
	}
}

void UObjectPoolManagerComponent::ClearPoolsTimed()
{
	ClearPools();
}

void UObjectPoolManagerComponent::PlayShootAt(FVector Location)
{
	if(IsValid(AudioComponent)) UGameplayStatics::PlaySoundAtLocation(this, AudioComponent->Sound, Location);
}

void UObjectPoolManagerComponent::WipePool()
{
	OnPoolWiped.Broadcast();
	UE_LOG(TABA_Logger, Warning, TEXT("%s: Doing a complete pool wipe"), *GetOwner()->GetActorLabel());
}

void UObjectPoolManagerComponent::PlayShoot_Implementation()
{
	IAudioInterface::PlayShoot_Implementation();
}


