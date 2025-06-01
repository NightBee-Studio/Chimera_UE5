
#include "TsPCGBreakMaterialAttribute.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"

#include "Helpers/PCGAsync.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"

#include "../TsBiomePCG.h"


//#include UE_INLINE_GENERATED_CPP_BY_NAME(PCGSnapToGround)

#define LOCTEXT_NAMESPACE "TsPCGBreakMaterialAttribute"

namespace PCGMetadataBreakTransformSettings
{
	inline EBiomeSType GetSType(const FTsPCGMaterial& m)
	{
		return m.mSType;
	}
	inline EBiomeMType GetMType(const FTsPCGMaterial& m)
	{
		return m.mMType;
	}
	inline float GetRatio(const FTsPCGMaterial& m)
	{
		return m.mRatio;
	}
}

void UPCGBreakMaterialSettings::PostLoad()
{
	Super::PostLoad();
}

FPCGAttributePropertyInputSelector UPCGBreakMaterialSettings::GetInputSource(uint32 Index) const
{
	return InputSource;
}

FName UPCGBreakMaterialSettings::GetOutputPinLabel(uint32 index) const
{
	switch (index){
	case 0:	return FName("SType");
	case 1:	return FName("MType");
	default:return FName("Ratio");
	}
}
//EBiomeSType				mSType;
//EBiomeMType				mMType;
//float						mRatio;


uint32 UPCGBreakMaterialSettings::GetResultNum() const
{
	return 3;
}

bool UPCGBreakMaterialSettings::IsSupportedInputType(uint16 TypeId, uint32 InputIndex, bool& bHasSpecialRequirement) const
{
	bHasSpecialRequirement = false;
	return PCG::Private::IsOfTypes<FTsPCGMaterial>(TypeId);
}

bool UPCGBreakMaterialSettings::HasDifferentOutputTypes() const
{
	return true;
}

TArray<uint16> UPCGBreakMaterialSettings::GetAllOutputTypes() const
{
	return { (uint16)EPCGMetadataTypes::Vector, (uint16)EPCGMetadataTypes::Quaternion, (uint16)EPCGMetadataTypes::Vector };
}

FName UPCGBreakMaterialSettings::GetOutputAttributeName(FName BaseName, uint32 Index) const
{
	if (BaseName != NAME_None){
		return FName(BaseName.ToString() + "." + GetOutputPinLabel(Index).ToString());
	}else{
		return NAME_None;
	}
}

#if WITH_EDITOR
FName UPCGBreakMaterialSettings::GetDefaultNodeName() const
{
	return TEXT("BreakPCGMaterialAttribute");
}

FText UPCGBreakMaterialSettings::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("UPCGBreakMaterialSettings", "NodeTitle", "Break TsPCGMaterial Attribute");
}
#endif // WITH_EDITOR

FPCGElementPtr UPCGBreakMaterialSettings::CreateElement() const
{
	return MakeShared<FPCGBreakMaterialElement>();
}


bool FPCGBreakMaterialElement::PrepareDataInternal(FPCGContext* Context) const
{ return true; }
bool FPCGBreakMaterialElement::ExecuteInternal(FPCGContext* Context) const
{ return true; }

bool FPCGBreakMaterialElement::DoOperation(PCGMetadataOps::FOperationData& OperationData) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGMetadataBreakVectorElement::Execute);

	const UPCGBreakMaterialSettings* Settings = static_cast<const UPCGBreakMaterialSettings*>(OperationData.Settings);
	check(Settings);

	return DoUnaryOp<FTsPCGMaterial>(
			OperationData,
			PCGMetadataBreakTransformSettings::GetSType,
			PCGMetadataBreakTransformSettings::GetMType,
			PCGMetadataBreakTransformSettings::GetRatio
		);
}



bool FPCGMetadataElementBase::PrepareDataInternal(FPCGContext* Context) const { return true;}
bool FPCGMetadataElementBase::ExecuteInternal(FPCGContext* Context) const { return true; }
