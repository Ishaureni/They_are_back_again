// Copyright Ónodi Kristóf. All Rights Reserved.


#include "Structures.h"

#include <string>


/*bool FIntCoords2D::operator==(const FIntCoords2D &arg1, const FIntCoords2D &arg2) const
{
	return (arg1.X == arg2.X)&&(arg1.Y == arg2.Y);
}*/
bool FGrid2D::IsContained(const TArray<FGrid2D>& InCoordsArray) const
{
	for(FGrid2D c:InCoordsArray)
	{
		if(c.X==X && c.Y==Y)
		{
			return true;
		}
	}
	return false;
}

bool FGrid2D::Equals(const FGrid2D& OtherCoord) const
{
	return (OtherCoord.X==X && OtherCoord.Y==Y);
}

bool FGrid2D::IsVerticallyAligned(const FGrid2D& OtherCoord) const
{
	return OtherCoord.X==X;
}

bool FGrid2D::IsHorizontallyAligned(const FGrid2D& OtherCoord) const
{
	return OtherCoord.Y==Y;
}

bool FGrid2D::IsStraight(const FGrid2D& OtherCoord) const
{
	return (IsVerticallyAligned(OtherCoord)||IsHorizontallyAligned(OtherCoord));
}

bool FGrid2D::IsBetween(const FGrid2D& A, const FGrid2D& B) const
{
	const bool bIsXBetween = (FMath::Min(A.X, B.X)<= X && X <= (FMath::Max(A.X, B.X)) && A.Y==B.Y==Y);
	const bool bIsYBetween = (FMath::Min(A.Y, B.Y)<= Y && Y <= (FMath::Max(A.Y, B.Y)) && A.X==B.X==X);
	return bIsXBetween || bIsYBetween;
}

int32 FGrid2D::LinearDistance(const FGrid2D& B) const
{
	const FGrid2D Dir = Subtract(B);
	return FMath::Abs(Dir.X) + FMath::Abs(Dir.Y);
}

FGrid2D FGrid2D::AddScaled(const FGrid2D& B, const int32 Scale) const
{
	FGrid2D ReturnGrid = FGrid2D(Scale * B.X, Scale * B.Y);
	ReturnGrid.X += X;
	ReturnGrid.Y += Y;
	return ReturnGrid;
}

FGrid2D FGrid2D::Subtract(const FGrid2D& B) const
{
	return FGrid2D(X-B.X, Y-B.Y);
}

FGrid2D FGrid2D::Unit() const
{
	FGrid2D C;
	C.X = X == 0 ? 0 : (X < 0 ? -1 : 1); 
	C.Y = Y == 0 ? 0 : (Y < 0 ? -1 : 1);
	return C;
}

EForwardDirection FGrid2D::GetForwardDirection() const
{
	const FGrid2D U = this->Unit();
	return U.Y == 1 ? Right : (U.Y == -1 ? Left : (U.X == -1 ? Down : Up));
}

FGrid2D FGrid2D::Clamp(const FGrid2D Min, const FGrid2D Max) const
{
	return FGrid2D(FMath::Clamp(X, Min.X, Max.X), FMath::Clamp(Y, Min.Y, Max.Y));
}

FGrid2D FGrid2D::Zero()
{
	return FGrid2D(0,0);
}

FString FGrid2D::ToString() const
{
	FString f = FString::FromInt(X) + ", " + FString::FromInt(Y) ;
	return f;
}

FGrid2D FGrid2D::Mix(const FGrid2D& OtherCoord, const bool bSwap) const
{
	if(bSwap) return FGrid2D(OtherCoord.X, Y);
	return FGrid2D(X, OtherCoord.Y);
}

FEnemyWave::FEnemyWave()
{
	EnemyAmount = 10;
	SpawnInterval = FDoubleRange().Inclusive(2, 5);
	InitialDelay = 3.0;
}

FObjectPool::FObjectPool()
{
	
}

FObjectPool::FObjectPool(const TSubclassOf<AActor> ItemClass)
{
	Name = ItemClass->GetFName();
	for(int i = 0; i < 3; ++i)
	{
		LastRemovedItemAmounts.Add(0);
	}
}

bool FObjectPool::HasAvailableItem() const
{
	return AvailablePool.Num()>0;
}

bool FObjectPool::IsClassOf(TSubclassOf<AActor> ItemClass) const
{
	return Name == ItemClass->GetFName();
}

void FObjectPool::AddItem(AActor* Item)
{
	AvailablePool.Add(Item);
}

void FObjectPool::CalculateReserve(const int32 CurrentRemovedAmount)
{
	int32 Avg = 0;
	LastRemovedItemAmounts.Sort();
	if(CurrentRemovedAmount == 0) LastRemovedItemAmounts[1]= 0;
	else if(CurrentRemovedAmount > 0) LastRemovedItemAmounts[0] = CurrentRemovedAmount;
	else
	{
		if(FMath::Abs(CurrentRemovedAmount)>LastRemovedItemAmounts[2])
		{
			LastRemovedItemAmounts[2] = FMath::Max(0, LastRemovedItemAmounts[2]-3);
		}
		else LastRemovedItemAmounts[2] = FMath::Abs(CurrentRemovedAmount);
	}
	for(int i = 0; i < LastRemovedItemAmounts.Num(); ++i)
	{
		Avg += LastRemovedItemAmounts[i];
	}
	Avg /= LastRemovedItemAmounts.Num();
	ExtraReserve = Avg;
}





