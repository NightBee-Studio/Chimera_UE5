#pragma once

#include "CoreMinimal.h"

#include "../Biome/TsBiomeMap.h"

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


enum EChannel{
	ER,
	EG,
	EB,
} ;

namespace TsBuilderTool
{
	UTexture2DArray*  Build_TexArray(
			TArray<UTexture2D*>&	textures ,
			const FString&			assetname
		);

	UTexture2D*		 Build_Texture(
			TMap<EChannel, UTexture2D*>&	textures ,
			const FString&			assetname
		);

	void	Build_MaterialSet(
			TArray<FTsGroundTex>&	texsets ,
			const FString&			assetname
		);

	UMaterialInstanceConstant*	Build_MaterialInstance(
		const FString& mm_path,
		const FString& assetname,
        const TMap<FName, UTexture2D*>& tex_table
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
