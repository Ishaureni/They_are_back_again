// Copyright Ónodi Kristóf. All Rights Reserved..


#include "Core/GM_TABA.h"

#include "GS_TABA.h"
#include "Kismet/GameplayStatics.h"
#include "TheyAreBackAgain/TheyAreBackAgain.h"

AGM_TABA::AGM_TABA()
{
	HUDClass = AHUDManagerBase::StaticClass();
	GameStateClass = AGS_TABA::StaticClass();
}

void AGM_TABA::BeginPlay()
{
	Super::BeginPlay();

	HUDManager =  Cast<AHUDManagerBase>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
}

bool AGM_TABA::OnSaveStarted_I_Implementation()
{
	HUDManager->OnSaveStarted();
	return true;
}

void AGM_TABA::OnSaveFinished_I_Implementation(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	HUDManager->OnSavingFinished(SlotName, UserIndex, bSuccess);
	
}

bool AGM_TABA::OnLoadStarted_I_Implementation(const FString& SlotName, const int32 UserIndex)
{
	HUDManager->OnLoadStarted(SlotName, UserIndex);
	return true;
}

void AGM_TABA::OnLoadFinished_I_Implementation(const bool bSuccess)
{
	ISaveLoadInterface::OnLoadFinished_I_Implementation(bSuccess);
	HUDManager->OnLoaded(bSuccess);
}

void AGM_TABA::EnemyArrivedToTargetObject_Implementation(AActor* Enemy)
{
	ILevelManagerInterface::EnemyArrivedToTargetObject_Implementation(Enemy);
}
