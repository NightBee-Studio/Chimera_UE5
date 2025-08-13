
#include "TsPCGSnapToGround.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Helpers/PCGAsync.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "PCGVolume.h"
#include "EngineUtils.h"   // TActorIterator

#define LOCTEXT_NAMESPACE "PCGSnapToGroundElement"


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
		
		if ( UWorld* world = Context ? Context->SourceComponent->GetWorld() : nullptr ) {
			FCollisionQueryParams params;
			params.bTraceComplex = true;
			params.AddIgnoredActor(Context->SourceComponent->GetOwner());
			for (TActorIterator<APCGVolume> it(world); it; ++it){
				params.AddIgnoredActor( *it );
			}

TArray<FName> hitnames ; 
			for (int ip = 0; ip < points.Num(); ip++) {
				out_points.Add(points[ip]);

				FTransform& trans = out_points[ip].Transform;
#define GAP	20000
				FVector from = trans.GetLocation() + FVector(0, 0, GAP);
				FVector to = trans.GetLocation() + FVector(0, 0, -GAP);
				FHitResult result;
				if (world->LineTraceSingleByChannel(result, from, to, ECC_WorldStatic, params)) {
					if ( result.GetActor() ) hitnames.AddUnique( *result.GetActor()->GetName()) ;
					//UE_LOG(LogTemp, Log, TEXT(" Pos=(%f %f %f)->Hit %f %f %f Actot:%s"),
					//	trans.GetLocation().X, trans.GetLocation().Y, trans.GetLocation().Z,
					//	result.Location.X, result.Location.Y, result.Location.Z, *result.GetActor()->GetName());
					trans.SetLocation(result.Location);
				}
			}

for( auto &nm : hitnames ){
	UE_LOG(LogTemp, Log, TEXT(" hitActot:%s"), *nm.ToString() ) ;
}

		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
