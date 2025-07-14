#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "../Util/TsImageMap.h"
#include "../Util/TsTextureMap.h"

#include "TsBiomeModel.generated.h"


USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeModel
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Textures"))
	TMap<TEnumAsByte<ETextureMap>,TObjectPtr<UTexture2D>>
									mTextures;
	TMap<TEnumAsByte<ETextureMap>,TsTextureMap*>
									mTextureMaps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Material"))
	TObjectPtr<UMaterialInstance>	mMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Model"   ))
	TObjectPtr<UStaticMesh>			mModel;

public:
	void Setup();

	void Lock() ;
	void UnLock() ;

	int GetSizeX() ;
	int GetSizeY() ;
};

USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeSpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeSurf" ))
	EBiomeSType				mSType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeGenre"))
	EBiomeGType				mGType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Models"   ))
	TArray<FTsBiomeModel>	mModels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Height"   ))
	float					mHeight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (DisplayName = "HeightRange"   ))
	FVector2D				mHeightRange;

public:
	static int	ID(EBiomeGType g, EBiomeSType s) { return (int)s + ((int)g << 8) ; }
	int			GetID() { return ID( mGType, mSType ) ; }
};


USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeModels
{
	GENERATED_BODY()
public:
	TMap<int,FTsBiomeSpec*> mSpecs ;

public:
	FTsBiomeModels() {}
	FTsBiomeModels( UDataTable* specs ) ;

	FTsBiomeModel*	GetRandomModel( int id ) ;
	FVector2D		GetHeightRange( int id ) ;

	void Lock() ;
	void UnLock() ;
};
