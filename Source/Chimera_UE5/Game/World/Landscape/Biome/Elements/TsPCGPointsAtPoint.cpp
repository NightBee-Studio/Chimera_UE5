
#include "TsPCGPointsAtPoint.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Helpers/PCGAsync.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"

#define LOCTEXT_NAMESPACE "PCGPointsAtPointElement"


FPCGElementPtr UPCGPointsAtPointSettings::CreateElement() const
{
	return MakeShared<FPCGPointsAtPointElement>();
}

bool FPCGPointsAtPointElement::ExecuteInternal(FPCGContext* Context) const// cannot change the name of 'Context'
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

		const UPCGPointsAtPointSettings* setting = Context->GetInputSettings<UPCGPointsAtPointSettings>();
		if (!setting) return true;

		const TArray<FPCGPoint>& points = pn_data->GetPoints();

		UPCGPointData*		out_data   = NewObject<UPCGPointData>();
		out_data->InitializeFromData(pn_data);
		TArray<FPCGPoint>&	out_points = out_data->GetMutablePoints();
		output.Data = out_data;
		
		for (int ip = 0; ip < points.Num(); ip++) {
			out_points.Add(points[ip]);

			if ( FMath::FRand() < setting->mCreateRatio ){
				int   n = setting->mPointNum + FMath::RandRange(-1,2) ;
				float ang  = 0 ;
				float step = PI*2 / n ;
				for (int cp=0; cp < n; cp++) {
					FPCGPoint point = points[ip] ;
					float   r = setting->mCreateRadius * FMath::FRandRange(0.8f,1.2f);
					float r_jit = FMath::FRandRange(-PI*0.1f, PI*0.1f);
					FVector v = r * FVector( FMath::Cos(ang+r_jit), FMath::Sin(ang+r_jit), 0.0f ) ;
					float r_ang = FMath::FRandRange(0.0f, 360.0f);
					float r_scl = FMath::FRandRange(0.5f, 0.8f);

					point.Transform.SetLocation( point.Transform.GetLocation() + v );
					point.Transform.SetRotation( FRotator(0,r_ang,0).Quaternion() );
					point.Transform.SetScale3D( FVector(r_scl) );

				//	UE_LOG(LogTemp, Log, TEXT(" %d scl%f ang%f "),cp, r_scl, r_ang ) ;

					out_points.Add( point );
					ang += step ;
				}
			}
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
