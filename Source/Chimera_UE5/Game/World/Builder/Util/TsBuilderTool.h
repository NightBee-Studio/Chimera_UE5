#pragma once

#include "CoreMinimal.h"

#include "../Biome/TsBiome.h"
#include "TsUtility.h"

#include "TsBuilderTool.generated.h"



USTRUCT(Blueprintable)
struct CHIMERA_UE5_API FTsGroundTex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "AlbedoMap"))
	TObjectPtr<UTexture2D>		mTexAlbedo ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "NormalMap"))
	TObjectPtr<UTexture2D>		mTexNormal ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "SpecularMap"))
	TObjectPtr<UTexture2D>		mTexSpecular ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "AmbOcclusionMap"))
	TObjectPtr<UTexture2D>		mTexAmbOcc ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "DisplacementMap"))
	TObjectPtr<UTexture2D>		mTexDisplacement ;
};

struct TsGroundSlot{
	UTexture2D	*	mTex ;
	EMaterialType	mMat ;
	TsGroundSlot(EMaterialType m, UTexture2D *t	): mTex(t),mMat(m) {} 
};

class TsBiomeMap ;
class TsNoiseMap;
class UMaterialInstanceConstant ;

namespace TsBuilderTool
{
	UTexture2DArray*  Build_TexArray(
			TArray<UTexture2D*>&	textures ,
			const FString&			assetname
		);

	UTexture2D*		 Combine_Texture(
			TMap<int, UTexture2D*>&	textures ,
			const FString&			assetname
		);

	void	Build_MaterialSet(
			TArray<FTsGroundTex>&	texsets
		);

	UMaterialInstanceConstant*	Build_MaterialInstance(
		const FString& mm_path,
		const FString& assetname,
        const TArray<TsGroundSlot>& slots
	);

	void	Build_HeightMesh(
			TsBiomeMap*				tex_map ,
			const TsUtil::TsBox &	tex_rect,
			TsNoiseMap *			noise_map,
			float					noise_scale,
			int						mesh_div,
			float					mesh_size,
			float					mesh_height,
			const FString&			asset_name,
			UMaterialInterface*		material = nullptr
		);
} ;
