#pragma once

#include "PCGSettings.h"
#include "PCGElement.h"
#include "TsPCGRangeSelection.generated.h"

UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGRangeSelectionSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
	UPCGRangeSelectionSettings(const FObjectInitializer& ObjectInitializer) {}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	int mIndex = 0;

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("RangeSelection"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "Node_RangeSelection", "Range Selection"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Param; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override {
		TArray<FPCGPinProperties> pins;
		pins.Emplace(TEXT("In"), EPCGDataType::Spatial);
		return pins;
	}
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override
	{
		TArray<FPCGPinProperties> pin;
		pin.Emplace(TEXT("Range"), EPCGDataType::Param);
		return pin;
	}
	virtual FPCGElementPtr CreateElement() const override;
};

class FPCGRangeSelectionElement
	: public IPCGElement
{
public:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

