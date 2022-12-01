// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataObjects/Structures.h"
#include "DataObjects/Enums.h"
#include "GridHelperLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UGridHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
private:
	
	
public:
	/**Returns constant of default tiles size*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static double GetGlobalGridSize(const UObject* WorldContextObject);

	/**Returns constant of default tiles size*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static double GetGridPadding(const UObject* WorldContextObject);

	/**Returns constant of default tiles size*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static double GetGridGlobalScale(const UObject* WorldContextObject);
		
	/**Returns global coordinates of a tile coord*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FVector GridPositionToWorld(const FGrid2D Pos,const UObject* WorldContextObject = nullptr);
	
	/**Returns global coordinates of a tile coord*/
	static FVector GridPositionToWorld(const int32 X, const int32 Y, const UObject* WorldContextObject = nullptr);
	
	/**Returns hit at world position 'Position'
	 * @param Position world position
	 * @param OutHit provide hit result reference to store hit info
	 * @return true if a hit happened
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
	static bool IsGridOccupiedInWorld(const UObject* WorldContextObject, const FVector Position, FHitResult& OutHit);

	/**Returns hit at world position 'Position'
	 * @param GridPosition position in grid coord
	 * @param OutHit provide hit result reference to store hit info
	 * @return true if a hit happened
	 */	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
	static bool IsGridOccupied(const UObject* WorldContextObject, const FGrid2D GridPosition, FHitResult& OutHit);
	static bool IsGridOccupied(const UObject* WorldContextObject, const FGrid2D GridPosition);

	static EForwardDirection GetCornerForwardDirection(const EForwardDirection& From, const EForwardDirection& To);
};
