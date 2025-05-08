#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"

#include "Biome/TsBiome.h"

#include "TsLandscape.generated.h"





// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(BlueprintType)
class CHIMERA_UE5_API ATsBiome
	: public ADynamicMeshActor
{
	GENERATED_BODY()

private:
	EBiomeType			mType;

	TArray<TsBiome*>	mBiome;// 

public:
	virtual ~ATsBiome() {}

	void			SetBiomeType(EBiomeType ty) { mType = ty; }
	EBiomeType		GetBiomeType() { return mType; }
};


// -------------------------------- UTsLandscape  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsLandscape///////////////////Maybe this is not used.....
	: public UObject
{
	GENERATED_BODY()

public:
	UTsLandscape();

	TArray<TObjectPtr<ATsBiome>>	mBiomes	;

#if		WITH_EDITOR
#endif	//WITH_EDITOR

};

