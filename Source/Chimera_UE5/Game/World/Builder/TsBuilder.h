#pragma once

#include "CoreMinimal.h"
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
	int			mSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Radius"))
	float		mRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "VoronoiSize"))
	float		mVoronoiSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "VoronoiJitter"))
	float		mVoronoiJitter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Reso"))
	int			mReso;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Erode Cycle"))
	int			mErodeCycle;

	UFUNCTION(BlueprintCallable)
	void		Build();
} ;

