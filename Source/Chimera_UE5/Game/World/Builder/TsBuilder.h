#pragma once

#include "CoreMinimal.h"

#include "Biome/TsBiome.h"
#include "Util/TsBuilderTool.h"

#include "Engine/DataTable.h"

#include "TsBuilder.generated.h"



UENUM(BlueprintType, meta = (Bitflags))
enum class EBuildMode : uint8 {
	EBM_None		= 0		UMETA(DisplayName = "None"),

	EBM_HeightMap	= 1<<0	UMETA(DisplayName = "HeightMap"),

	EBM_MaterialMap	= 1<<1	UMETA(DisplayName = "MaterialMap"),
	EBM_StaticMesh	= 1<<2	UMETA(DisplayName = "StaticMesh"),
	EBM_UpdateRatio	= 1<<3	UMETA(DisplayName = "UpdateRatio"),
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EBuildMode", DisplayName = "Mode"))
	int32					mMode ;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Texture Sets"))
	TMap<TEnumAsByte<EMaterialType>,FTsGroundTex>	mMaterialSets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Heightmap"))
	UTexture2D*				mHeightmap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Texture Map"))
	TMap<TEnumAsByte<ETextureMap>,TObjectPtr<UTexture2D>>	mTextureMaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Biome Specs"))
	UDataTable*				mBiomeTable;

	UFUNCTION(BlueprintCallable)
	void		Build();
} ;

