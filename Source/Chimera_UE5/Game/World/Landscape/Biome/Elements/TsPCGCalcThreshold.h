#pragma once

#include "CoreMinimal.h"
#include "Elements/Metadata/PCGMetadataOpElementBase.h"

#include "../TsBiomePCG.h"


#include "TsPCGCalcThreshold.generated.h"


UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural))
class UPCGCalcThresholdSettings
	: public UPCGMetadataSettingsBase
{
	GENERATED_BODY()

public:
	// ~Begin UObject interface
	virtual void PostLoad() override;
	// ~End UObject interface
	// 
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override;
	virtual FText GetDefaultNodeTitle() const override;
#endif
	virtual FPCGAttributePropertyInputSelector GetInputSource(uint32 Index) const override;

	virtual FName GetOutputPinLabel(uint32 Index) const override;
	virtual uint32 GetResultNum() const override;

	virtual bool IsSupportedInputType(uint16 TypeId, uint32 InputIndex, bool& bHasSpecialRequirement) const override;
	virtual FName GetOutputAttributeName(FName BaseName, uint32 Index) const override;

	virtual bool HasDifferentOutputTypes() const override;
	virtual TArray<uint16> GetAllOutputTypes() const override;

protected:
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Input, meta = (PCG_Overridable))
	FPCGAttributePropertyInputSelector InputSource;



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	UTexture2D*							mTexture;
};


class FPCGCalcThresholdElement
	: public FPCGMetadataElementBase
{
	TArray<float>	mResult;

public:
	FPCGCalcThresholdElement();
protected:

	static float Get0(const TArray<FTsPCGMaterial>& m);
	static float GetR1(const TArray<FTsPCGMaterial>& m);
	static float GetR2(const TArray<FTsPCGMaterial>& m);
	static float GetR3(const TArray<FTsPCGMaterial>& m);
	static float GetR4(const TArray<FTsPCGMaterial>& m);
	static float Get1(const TArray<FTsPCGMaterial>& m);

	virtual bool PrepareDataInternal(FPCGContext* Context) const override;
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	virtual bool DoOperation(PCGMetadataOps::FOperationData& op_data) const override;
};

