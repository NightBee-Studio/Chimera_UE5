#include "TsPCGRangeSelection.h"

#include "PCGParamData.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Metadata/Accessors/IPCGAttributeAccessor.h"
#include "Metadata/Accessors/PCGAttributeAccessorKeys.h"
#include "Metadata/Accessors/PCGAttributeAccessorHelpers.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"


FPCGElementPtr UPCGRangeSelectionSettings::CreateElement() const
{
	return MakeShared<FPCGRangeSelectionElement>();
}

bool FPCGRangeSelectionElement::ExecuteInternal(FPCGContext* Context) const
{
	const TArray<FPCGTaggedData>& inputs  = Context->InputData. TaggedData;
	      TArray<FPCGTaggedData>& outputs = Context->OutputData.TaggedData;

	for (const auto& input : inputs) {
		const UPCGSpatialData* sp_data = Cast<UPCGSpatialData>(input.Data);
		const UPCGPointData*   pn_data = Cast<UPCGPointData  >(sp_data   );
		if (!pn_data) {
			UE_LOG(LogTemp, Error, TEXT("Input is not point data"));
			continue;
		}

		auto ExtractAttribute = [&](const FName& name) -> float {
				FPCGAttributePropertySelector selector;
				selector.SetAttributeName(name);
				TUniquePtr<const IPCGAttributeAccessor    > accessor = PCGAttributeAccessorHelpers::CreateConstAccessor(pn_data, selector);
				TUniquePtr<const IPCGAttributeAccessorKeys> keys     = PCGAttributeAccessorHelpers::CreateConstKeys    (pn_data, selector);
				if (!accessor || !keys) {
					UE_LOG(LogTemp, Warning, TEXT("Attribute %s not found or empty"), *name.ToString());
					return 0.0f;
				}
				float value = 0.0f;
				if (!accessor->Get<float>(value, *keys)) {
					UE_LOG(LogTemp, Warning, TEXT("Failed to read attribute %s"), *name.ToString());
				}
				return value;
			};

		//for (const FName& at_name : { TEXT("R1"), TEXT("R2"), TEXT("R3"), TEXT("R4") }) {
		//	UE_LOG(LogTemp, Log, TEXT(" Read %s = %.2f"), *at_name.ToString(), ExtractAttribute(at_name) );
		//}

		const UPCGRangeSelectionSettings* setting = Context->GetInputSettings<UPCGRangeSelectionSettings>();
		if (!setting) return true;

		int i = setting->mIndex-1 ;
		TArray<FName> attr_names = { TEXT("R1"), TEXT("R2"), TEXT("R3"), TEXT("R4") };
		FVector2D range(
			i>=0 ? ExtractAttribute(attr_names[i  ]) : 0.0f,
			i< 3 ? ExtractAttribute(attr_names[i+1]) : 1.0f
		);
		UE_LOG(LogTemp, Log, TEXT(" Range %.2f %.2f"), range.X, range.Y);

		UPCGParamData* out_data = NewObject<UPCGParamData>();
		out_data->Metadata->CreateAttribute<FVector2D>(TEXT("Range"), range, /*bAllowOverride=*/false, /*bOverrideParent=*/false);
		out_data->Metadata->AddEntry(); // 1çsí«â¡

		FPCGTaggedData td = { .Data = out_data, .Pin = TEXT("Range"),  };
		outputs.Add(MoveTemp(td));
	}

	return true;
}
