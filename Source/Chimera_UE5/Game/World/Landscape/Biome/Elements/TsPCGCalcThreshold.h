#pragma once

#include "CoreMinimal.h"
#include "Elements/Metadata/PCGMetadataOpElementBase.h"

#include "../TsBiomePCG.h"


#include "TsPCGCalcThreshold.generated.h"


UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGCalcThresholdSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("CalcThreshold"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "CalcThreshold", "Calc Threshold"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Param; }
#endif

	//virtual bool HasInputPin() const override { return false; }

	virtual TArray<FPCGPinProperties> InputPinProperties() const override {
		return Super::DefaultPointInputPinProperties();
	}
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override{
		TArray<FPCGPinProperties> Pins;
		Pins.Emplace(TEXT("R1"), EPCGDataType::Param);
		Pins.Emplace(TEXT("R2"), EPCGDataType::Param);
		Pins.Emplace(TEXT("R3"), EPCGDataType::Param);
		Pins.Emplace(TEXT("R4"), EPCGDataType::Param);
		return Pins;
	}

protected:
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	TArray<FTsPCGMaterial>	mLayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	UTexture2D*				mTexture;
};


class FPCGCalcThresholdElement
	: public IPCGElement
{
public:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

