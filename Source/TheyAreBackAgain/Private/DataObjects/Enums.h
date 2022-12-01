#pragma once

#include "CoreMinimal.h"
#include "Enums.generated.h"

UENUM(BlueprintType)
enum EGridConnections
{
	NS,
	EW,
	NE,
	ES,
	SW,
	WN,
	NSE,
	NSW,
	EWN,
	EWS,
	NSEW
};

UENUM(BlueprintType)
enum ETileType
{
	Undefined,
	Base,
	Target,
	Spawner,
	RoadStraight,
	RoadCorner,
	RoadSplit,
	RoadCrossing,
	RiverStraight,
	RiverCorner,
	RiverSplit,
	RiverCrossing,
	Bridge,
	Scenery,
	Count
};
ENUM_RANGE_BY_COUNT(ETileType, ETileType::Count)



UENUM()
enum EForwardDirection
{
	Up = 0,
	Right = 1,
	Down = 2,
	Left = 3,
};
ENUM_RANGE_BY_COUNT(EForwardDirection, EForwardDirection::Left)
