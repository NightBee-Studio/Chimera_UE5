#pragma once


#include "CoreMinimal.h"

#include "PCGSettings.h"

#include "TsPCGPointsAtPoint.generated.h"



// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGPointsAtPointSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName("PointsAtPoint"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "PointsAtPoint", "Points At Point"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }

protected:
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float		mCreateRatio  = 0.3f ;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float		mCreateRadius = 100.0f ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	FVector2D	mScaleRange = FVector2D(0.7f,1.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	int			mPointNum     = 5 ;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float		mPointRadius  = 50.0f;
};

class FPCGPointsAtPointElement
	: public IPCGElement
{
public:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

