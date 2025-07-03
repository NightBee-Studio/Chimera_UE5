#pragma once


#include "CoreMinimal.h"

#include "PCGSettings.h"

#include "TsPCGPlaneSampler.generated.h"



// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGPlaneSamplerSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName("PlaneSampler"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "PlaneSampler", "Plane Sampler"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Sampler; }
	virtual FText GetNodeTooltipText() const override { return FText::FromString(TEXT("Samples a 2D XY plane using bounding volume and spacing with random density.")); }
#endif
	//virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> InputPinProperties() const override {
		TArray<FPCGPinProperties> pins;
		pins.Emplace(TEXT("Volume"), EPCGDataType::Spatial);
		return pins;
	}
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }

	UPROPERTY(EditAnywhere, Category = Settings, meta = (PCG_Overridable, DisplayName ="VoxelSize" ))
	FVector	mVoxelSize = FVector(100,100,1);
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PCG_Overridable, DisplayName ="ObjectSize" ))
	FVector	mObjectSize = FVector(100,100,1);
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PCG_Overridable, DisplayName ="PlaneZ" ))
	float	mPlaneZ = 0.0f;
	UPROPERTY(EditAnywhere, Category = Settings, meta = (PCG_Overridable, DisplayName ="Seed" ))
	int32	mSeed = 1234 ;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};


class FPCGPlaneSamplerElement
	: public IPCGElement
{
public:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

