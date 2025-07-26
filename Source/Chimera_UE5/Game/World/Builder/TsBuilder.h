#pragma once

#include "CoreMinimal.h"

#include "Biome/TsBiome.h"

#include "Engine/DataTable.h"

#include "TsBuilder.generated.h"



UENUM(BlueprintType)
enum EBuildMode {
	EBM_Heightmap,
	EBM_FullGrid,
} ;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Mode"))
	TEnumAsByte<EBuildMode>	mMode ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Seed"))
	int						mSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Island", Meta = (DisplayName = "Radius"))
	float					mRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Island", Meta = (DisplayName = "VoronoiSize"))
	float					mVoronoiSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Island", Meta = (DisplayName = "VoronoiJitter"))
	float					mVoronoiJitter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Reso"))
	int						mReso;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Erode Cycle"))
	int						mErodeCycle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Unit Size"))
	int						mUnitSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Unit Division"))
	int						mUnitDiv;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Unit Reso"))
	int						mUnitReso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Heightmap"))
	UTexture2D*				mHeightmap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Texture Map"))
	TMap<TEnumAsByte<ETextureMap>,TObjectPtr<UTexture2D>>	mTextureMaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Biome Specs"))
	UDataTable*				mBiomeTable;

	UFUNCTION(BlueprintCallable)
	void		Build();
} ;

