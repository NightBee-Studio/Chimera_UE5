#pragma once

#include "CoreMinimal.h"
#include "TsTextureMap.h"

class TsHeightMesh 
{
public:
	void Build(UObject* outer, TsTextureMap* heightmap, int div, const FString& packagename, const FString& assetname );
};


