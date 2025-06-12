#pragma once

#include "CoreMinimal.h"
#include "Elements/Metadata/PCGMetadataOpElementBase.h"

#include "../TsBiomePCG.h"

#include "TsPCGRangeCalculation.generated.h"


UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGRangeCalculationSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("RangeCalculation"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "Node_RangeCalculation", "Range Calculation"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Param; }
#endif

	//virtual bool HasInputPin() const override { return false; }

	virtual TArray<FPCGPinProperties> InputPinProperties() const override {
		TArray<FPCGPinProperties> pins;
		pins.Emplace(TEXT("In"), EPCGDataType::Spatial);
		return pins;
	}
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override{
		TArray<FPCGPinProperties> pins;
		pins.Emplace(TEXT("Out"), EPCGDataType::Spatial);
		return pins;
	}

protected:
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	TArray<FTsPCGMaterial>	mLayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	UTexture2D*				mTexture;
};


class FPCGRangeCalculationElement
	: public IPCGElement
{
public:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

