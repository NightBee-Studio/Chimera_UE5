#pragma once


#include "CoreMinimal.h"

#include "PCGSettings.h"

#include "TsPCGFilterTexture.generated.h"



// -------------------------------- Biome --------------------------------
//
//
//

UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGFilterTextureSettings
	: public UPCGSettings
{
	GENERATED_BODY()

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName("TsFilterTexture"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCG", "Filtered the Postions by Texture ", "Filtered By Texture"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps;}
#endif
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }

protected:
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	FVector2D					mRange;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	TObjectPtr<UTexture2D>		mTexture;
};


class FPCGFilterTextureElement
	: public IPCGElement
{
private:
	int		GetIndexFromPos(const FPCGContext* context, const FPCGPoint& point )const ;

public:
	virtual bool ExecuteInternal( FPCGContext* context ) const override;
};

