#pragma once


#include "CoreMinimal.h"

#include "PCGSettings.h"

#include "TsPCGFilterTexture.generated.h"



// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGFilterTextureSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName("FilterTexture"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "Filtered the Postions by Texture ", "Filtered By Texture"); }
	//virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Debug; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties()  const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

protected:
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float			mThresholdMin;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float			mThresholdMax;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	UTexture2D*		mTexture;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	FVector			mVolumeSize;
};


class FPCGFilterTextureElement
	: public IPCGElement
{
private:
	TObjectPtr<UTexture2D>	mTexture ;
	FVector					mVolumeSize;
	float					mMin, mMax;

	int		GetIndexFromPos(const FPCGContext* context, const FPCGPoint& point )const ;

public:
	FPCGFilterTextureElement(TObjectPtr<UTexture2D> texture, const FVector& size, float min, float max);

	virtual bool ExecuteInternal( FPCGContext* context ) const override;
};

