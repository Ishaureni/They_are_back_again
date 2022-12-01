// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TileSetDA.h"
#include "Engine/DataAsset.h"

#include "DataObjects/Structures.h"

#include "LevelDA.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ULevelDA : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * Random stream for predictable random generation
	 * e.g. generating scenery tiles
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InitialSeed = 21;
	
	/**- Width of the map
	 * - Unit is in tiles
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin=5))
	int32 N = 10;

	/**- Height of the map
	 * - Unit is in tiles
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin=5))
	int32 M = 10;

	/**- Tile set DataAsset
	 *
	 * Determines the visual of the map.
	 * At least 'BaseTileMeshPtr' must be valid inside.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTileSetDA* TileSet;

	/** - Add default enemies to be loaded that all lanes can use
	 *  - Ground enemies will be marked and only ground lanes can use them, if any
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Level setting")
	TArray<UEnemyBaseDA*> DefaultLevelEnemies;

	
	/**- Scenery tiles chance to spawn
	 * - Replaces base tiles with scenery
	 * - Between 0 and 0.75
	 * - If too high there will be no place to build
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin=0, UIMax=0.75))
	double SceneryTileChance = 0.2;
	
	/**- Enemy lanes*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FEnemyLane> Lanes;

	/**- Target tile coord
	 * - Clamps between FGrid2D(0,0) and FGrid2D(N , M)
	 *
	 * Target coord, that must be defended. 
	 * Acts as every enemy lane's last waypoint.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGrid2D TargetCoord;

	
};
