#pragma once

#include "CoreMinimal.h"

#include "Biome/TsBiome.h"

#include "Engine/DataTable.h"

#include "TsBuilder.generated.h"



// -------------------------------- UTsLandscape  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API ATsBuilder
	: public AActor
{
	GENERATED_BODY()

private:
	void*		mImplement ;

public:
	ATsBuilder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Seed"))
	int				mSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Island", Meta = (DisplayName = "Radius"))
	float			mRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Island", Meta = (DisplayName = "VoronoiSize"))
	float			mVoronoiSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Island", Meta = (DisplayName = "VoronoiJitter"))
	float			mVoronoiJitter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Reso"))
	int				mReso;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Erode Cycle"))
	int				mErodeCycle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Biome Specs"))
	UDataTable*		mBiomeTable;

	UFUNCTION(BlueprintCallable)
	void		Build();
} ;

