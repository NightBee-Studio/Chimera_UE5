#pragma once


#include "CoreMinimal.h"

//#include "PCGSettings.h"
#include "PCGGraph.h"
#include "../../Builder/Biome/TsBiome.h"

#include "TsBiomePCG.generated.h"


USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsPCGModel
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Models"))
	TArray<UStaticMesh*>	mModels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Size"))
	FVector					mSize;
};


USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsPCGMaterial
{
	GENERATED_BODY()
public:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "SType"))
	//EBiomeSType				mSType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "MType"))
	EBiomeMType				mMType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Ratio"))
	float					mRatio;
};

// -------------------------------- Biome --------------------------------
//
//
//

