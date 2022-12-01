// Copyright Ónodi Kristóf. All Rights Reserved.


#include "Libraries/GridHelperLibrary.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Core/GI_TABA.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"

double UGridHelperLibrary::GetGlobalGridSize(const UObject* WorldContextObject)
{
	if(IsValid(WorldContextObject)) return Cast<UGI_TABA>(UGameplayStatics::GetGameInstance(WorldContextObject))->GetGlobalGridSize();
	return 100.0;
}

double UGridHelperLibrary::GetGridPadding(const UObject* WorldContextObject)
{
	if(IsValid(WorldContextObject)) return Cast<UGI_TABA>(UGameplayStatics::GetGameInstance(WorldContextObject))->GetGlobalGridPadding();
	return 10.0;
}

double UGridHelperLibrary::GetGridGlobalScale(const UObject* WorldContextObject)
{
	if(IsValid(WorldContextObject)) return Cast<UGI_TABA>(UGameplayStatics::GetGameInstance(WorldContextObject))->GetGlobalScalingFactor();
	
	return 1.0;
}

FVector UGridHelperLibrary::GridPositionToWorld(const FGrid2D Pos,const UObject* WorldContextObject)
{
	return GridPositionToWorld(Pos.X,Pos.Y, WorldContextObject);
}

FVector UGridHelperLibrary::GridPositionToWorld(const int32 X, const int32 Y,const UObject* WorldContextObject)
{
	
	return FVector(X*(GetGlobalGridSize(WorldContextObject)+GetGridPadding(WorldContextObject)), Y*(GetGlobalGridSize(WorldContextObject)+GetGridPadding(WorldContextObject)), 0.0)*GetGridGlobalScale(WorldContextObject);
}

bool UGridHelperLibrary::IsGridOccupiedInWorld(const UObject* WorldContextObject, const FVector Position, FHitResult& OutHit)
{
	const UWorld* WorldContext = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!WorldContext)
	{
		UE_LOG(TABA_Logger, Warning, TEXT("World not found for casting ray"));
		return false;
	}

	const FVector Ray = FVector(0.0, 0.0, 1000.0);

	return WorldContext->LineTraceSingleByChannel(OutHit, Position+(Ray/2), Position-(Ray/2), ECC_GameTraceChannel1); //ECC_GameTraceChannel1 = "LevelTile" ECC defined in Editor / DefaultEngine.ini
}

bool UGridHelperLibrary::IsGridOccupied(const UObject* WorldContextObject, const FGrid2D GridPosition, FHitResult& OutHit)
{
	return IsGridOccupiedInWorld(WorldContextObject, GridPositionToWorld(GridPosition, WorldContextObject), OutHit);
}

bool UGridHelperLibrary::IsGridOccupied(const UObject* WorldContextObject, const FGrid2D GridPosition)
{
	FHitResult Hit;
	return IsGridOccupiedInWorld(WorldContextObject, GridPositionToWorld(GridPosition, WorldContextObject), Hit);
}

EForwardDirection UGridHelperLibrary::GetCornerForwardDirection(const EForwardDirection& From,	const EForwardDirection& To)
{
	EForwardDirection d = From;
	
	if (To - From != 1 && To - From != -3)
	{
		d =  static_cast<EForwardDirection>((From + 1 ) % 4);
	}
	
	return d;
}



