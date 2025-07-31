#pragma once

#include "CoreMinimal.h"
//#include "TsTextureMap.h"
//#include "TsUtility.h"

#include "Materials/MaterialInstanceConstant.h"

//class UMaterialInstanceConstant ;

class TsMaterial 
{
public:
	static 
	UMaterialInstanceConstant* Build(
		const FString& mm_path,
		const FString& assetname,
        const TMap<FName, UTexture2D*>& tex_table
	);
};

