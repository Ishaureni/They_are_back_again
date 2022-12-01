// Copyright Ónodi Kristóf. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TileSetDA.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class UTileSetDA : public UDataAsset
{
	GENERATED_BODY()

public:

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	/**	- Base tile mesh
	 *	- Buildable
	 *
	 * Mandatory, level won't load without this.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> BaseTileMeshPtr;

	/**	- Spawner mesh tile
	*
	* Advised, so spawner tile can be visualized. 
	* At spawner position there will be an empty base tile, if left empty.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> SpawnerTileMeshPtr;
	
	/**	- Target tile mesh
	*	- Not buildable
	*
	* Advised, so target tile is visualized correctly. 
	* Defaults to 'BaseTileMeshPtr', marked with not buildable, if left empty.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> TargetTileMeshPtr;

	/**	- Straight road tile mesh
	*	- Not buildable
	* 
	* Should be assigned if any ground lane present. 
	* Defaults to 'BaseTileMeshPtr', marked with not buildable, if left empty.
	* Dir: -X -> +X
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> StraightRoadTileMeshPtr;
	
	/**	- Corner road tile mesh
	*	- Not buildable
	* 
	* Should be assigned if any ground lane present. 
	* Defaults to 'BaseTileMeshPtr', marked with not buildable, if left empty.
	* Dir: -X -> +Y
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> CornerRoadTileMeshPtr;
	
	/**	- Crossroad tile mesh
	*	- Not buildable
	* 
	* Should be assigned if any ground lane present. 
	* Defaults to 'BaseTileMeshPtr', marked with not buildable, if left empty.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> CrossRoadTileMeshPtr;
	
	/**	- Split road tile mesh
	*	- Not buildable
	* 
	* Should be assigned if any ground lane present. 
	* Defaults to 'BaseTileMeshPtr', marked with not buildable, if left empty.
	* Dir: -X -> +X AND -> +Y
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> SplitRoadTileMeshPtr;
	
	/**	- Scenery tile mesh array
	*	- Not buildable
	*
	* Optional. 
	* Should be assigned if you want to spawn scenery tiles as challenges.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TSoftObjectPtr<UStaticMesh>> SceneryTileMeshPtrs;

	/**- Bordering tile mesh
	 * - Optional - visual purpose only
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UStaticMesh> LevelBorderTileMeshPtr;
	
};
