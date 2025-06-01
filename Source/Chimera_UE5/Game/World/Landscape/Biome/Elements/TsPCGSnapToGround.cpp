
#include "TsPCGSnapToGround.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"

#include "Helpers/PCGAsync.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"


//#include UE_INLINE_GENERATED_CPP_BY_NAME(PCGSnapToGround)

#define LOCTEXT_NAMESPACE "PCGSnapToGroundElement"


TArray<FPCGPinProperties> UPCGSnapToGroundSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);

	return PinProperties;
}

TArray<FPCGPinProperties> UPCGSnapToGroundSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);

	return PinProperties;
}

FPCGElementPtr UPCGSnapToGroundSettings::CreateElement() const
{
	return MakeShared<FPCGSnapToGroundElement>();
}

bool FPCGSnapToGroundElement::ExecuteInternal(FPCGContext* Context) const// cannot change the name of 'Context'
{
	const TArray<FPCGTaggedData>	inputs	= Context->InputData.GetInputs();
	TArray<FPCGTaggedData>&			outputs	= Context->OutputData.TaggedData;

	for (const FPCGTaggedData& i : inputs){
		TRACE_CPUPROFILER_EVENT_SCOPE( FPCGTransformPointsElement::Execute::InputLoop );
		FPCGTaggedData&		output = outputs.Add_GetRef(i);

		const UPCGSpatialData* sp_data = Cast<UPCGSpatialData>(i.Data);
		if (!sp_data){
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingSpatialData", "Unable to get Spatial data from input"));
			continue;
		}

		const UPCGPointData* pn_data = sp_data->ToPointData(Context);
		if (!pn_data){
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingPointData", "Unable to get Point data from input"));
			continue;
		}

		const TArray<FPCGPoint>& points = pn_data->GetPoints();

		UPCGPointData*		out_data   = NewObject<UPCGPointData>();
		out_data->InitializeFromData(pn_data);
		TArray<FPCGPoint>&	out_points = out_data->GetMutablePoints();
		output.Data = out_data;

		FPCGAsync::AsyncPointProcessing(
			Context, points.Num(), out_points,
			[&](int32 idx, FPCGPoint& out_point) {
				out_point = points[idx];
				FTransform& trans = out_point.Transform;
#define GAP	2000
				FVector from = trans.GetLocation() + FVector(0, 0, GAP);
				FVector to   = trans.GetLocation() + FVector(0, 0,-GAP);
				FHitResult result;

				UWorld* world = Context ? Context->SourceComponent->GetWorld() : nullptr;
				if (world && world->LineTraceSingleByChannel(result, from, to, ECC_Visibility, FCollisionQueryParams())) {
					trans.SetLocation( result.Location );
				}
				return true;
			});
	}

	return true;
}
