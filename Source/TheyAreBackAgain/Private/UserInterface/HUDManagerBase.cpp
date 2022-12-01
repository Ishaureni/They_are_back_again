// Copyright Ónodi Kristóf. All Rights Reserved.


#include "UserInterface/HUDManagerBase.h"

#include "TheyAreBackAgain/TheyAreBackAgain.h"

#pragma region Save - Load

#pragma region Save
void AHUDManagerBase::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);

	GameInstance = GetGameInstance();
	if(!IsValid(GameInstance))
	{
		UE_LOG(TABA_Logger, Warning, TEXT("%s: Couldn't get UGI_TABA gameinstance."))
	}
}

void AHUDManagerBase::OnSaveStarted()
{
	//TODO start anim
}

void AHUDManagerBase::OnSavingFinished(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	//TODO display text

	if(bSuccess)
	{
		//Print save succeded
		return;
	}
	else
	{
		//Print save failed
	}
}

#pragma endregion Save

#pragma region Load

void AHUDManagerBase::OnLoadStarted(const FString& SlotName, const int32 UserIndex)
{
	//TODO start anim

}

void AHUDManagerBase::OnLoaded(bool bSuccess)
{
	//TODO display text

	if(bSuccess)
	{
		//Print load succeded
		return;
	}
	else
	{
		//Print load failed
	}
}

#pragma endregion Load

#pragma endregion Save - Load
