#include "TsPCGCalcThreshold.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"

#include "../TsBiomePCG.h"


#define LOCTEXT_NAMESPACE "TsPCGCalcThreshold"


void UPCGCalcThresholdSettings::PostLoad()
{
	Super::PostLoad();
}

FPCGAttributePropertyInputSelector UPCGCalcThresholdSettings::GetInputSource(uint32 Index) const
{
	return InputSource;
}

FName UPCGCalcThresholdSettings::GetOutputPinLabel(uint32 index) const
{
	switch (index) {
	case 0:	return FName("R1");
	case 1:	return FName("R2");
	case 2:	return FName("R3");
	default:return FName("R4");
	}
}

uint32 UPCGCalcThresholdSettings::GetResultNum() const
{
	return 4;
}

bool UPCGCalcThresholdSettings::IsSupportedInputType(uint16 TypeId, uint32 InputIndex, bool& bHasSpecialRequirement) const
{
	bHasSpecialRequirement = false;
	return true ;
}

bool UPCGCalcThresholdSettings::HasDifferentOutputTypes() const
{
	return true;
}

TArray<uint16> UPCGCalcThresholdSettings::GetAllOutputTypes() const
{
	return {	(uint16)EPCGMetadataTypes::Float, 
				(uint16)EPCGMetadataTypes::Float,
				(uint16)EPCGMetadataTypes::Float,
				(uint16)EPCGMetadataTypes::Float, };
}

FName UPCGCalcThresholdSettings::GetOutputAttributeName(FName BaseName, uint32 Index) const
{
	if (BaseName != NAME_None) {
		return FName(BaseName.ToString() + "." + GetOutputPinLabel(Index).ToString());
	} else {
		return NAME_None;
	}
}

#if WITH_EDITOR
FName UPCGCalcThresholdSettings::GetDefaultNodeName() const
{
	return TEXT("CalcThreshold");
}

FText UPCGCalcThresholdSettings::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("UPCGCalcThresholdSettings", "NodeTitle", "CalcThreshold");
}
#endif // WITH_EDITOR

FPCGElementPtr UPCGCalcThresholdSettings::CreateElement() const
{
	return MakeShared<FPCGCalcThresholdElement>();
}









FPCGCalcThresholdElement::FPCGCalcThresholdElement()
{
	mResult = {1,2,3,4,5};
}

bool FPCGCalcThresholdElement::PrepareDataInternal(FPCGContext* Context) const
{
	return true;
}
bool FPCGCalcThresholdElement::ExecuteInternal(FPCGContext* Context) const
{
	return true;
}


float FPCGCalcThresholdElement::GetR1(const TArray<FTsPCGMaterial>& m)
{
	return 0.1f;
}
float FPCGCalcThresholdElement::GetR2(const TArray<FTsPCGMaterial>& m)
{
	return 0.2f;
}
float FPCGCalcThresholdElement::GetR3(const TArray<FTsPCGMaterial>& m)
{
	return 0.3f; 
}
float FPCGCalcThresholdElement::GetR4(const TArray<FTsPCGMaterial>& m) {
	return 0.4f; 
}

bool FPCGCalcThresholdElement::DoOperation(PCGMetadataOps::FOperationData& op_data) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCalcThresholdElement::Execute);

	const UPCGCalcThresholdSettings* settings = static_cast<const UPCGCalcThresholdSettings*>(op_data.Settings);
	check(settings);

	return 
		DoUnaryOp<TArray<FTsPCGMaterial>>(
				op_data,
				GetR1, GetR2, GetR3, GetR4
			);
}

