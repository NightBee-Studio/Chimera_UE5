#pragma once


#include "CoreMinimal.h"

#include "PCGSettings.h"

#include "TsPCGSnapToGround.generated.h"



// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGSnapToGroundSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName("SnapToGround"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "SnapToGround", "Snap To Ground"); }
	//virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Debug; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

protected:
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float mLineTraceOffset = 1000;
};

class FPCGSnapToGroundElement
	: public IPCGElement
{
public:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

