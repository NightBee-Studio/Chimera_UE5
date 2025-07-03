#include "TsUtility.h"
#include "CoreMinimal.h"



void	TsUtil::RandSeed(int i) {
	FMath::RandInit( i );
}

int32	TsUtil::RandRange(int32 a, int32 b)
{
	return FMath::RandRange(a, b);
}

float	TsUtil::RandRange(float a, float b) {
	return FMath::RandRange(a, b);
}

float	TsUtil::RandRange(const FVector2D &v){
	return FMath::RandRange(v.X, v.Y);
}
