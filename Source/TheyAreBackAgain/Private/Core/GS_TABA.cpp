// Copyright Ónodi Kristóf. All Rights Reserved..


#include "Core/GS_TABA.h"

void AGS_TABA::OnEnemyDied_Implementation(bool bIsFlyingEnemy, FTransform DiedAt)
{
	bIsFlyingEnemy?++FlyingKilled:++GroundKilled;
	OnEnemyDiedEvent.Broadcast(bIsFlyingEnemy, DiedAt);
}
