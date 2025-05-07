#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"

#include "Land/TsArea.h"

#include "TsLandscape.generated.h"





// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(BlueprintType)
class CHIMERA_UE5_API ATsLandArea
	: public ADynamicMeshActor
{
	GENERATED_BODY()

private:
	EBiomeType		mType;
	EBiomeSrfType	mSurfType;//Å@Ç¢ÇÁÇ»Ç¢

	TsBiome	*		mBiome;// 

public:
	virtual ~ATsLandArea() {}

	void			SetBiomeType(EBiomeType ty) { mType = ty; }
	EBiomeType		GetBiomeType() { return mType; }
	void			SetBiomeSrfType(EBiomeSrfType ty) { mSurfType = ty; }
	EBiomeSrfType	GetBiomeSrfType() { return mSurfType; }
};


// -------------------------------- UTsLandscape  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsLandscape
	: public UObject
{
	GENERATED_BODY()

public:
	UTsLandscape();

	TArray<TObjectPtr<ATsLandArea>>	mAreas	;


#if		WITH_EDITOR
#endif	//WITH_EDITOR

};

