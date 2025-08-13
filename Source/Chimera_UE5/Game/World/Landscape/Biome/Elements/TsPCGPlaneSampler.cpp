
#include "TsPCGPlaneSampler.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Helpers/PCGAsync.h"
#include "Helpers/PCGHelpers.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Metadata/PCGMetadata.h"
#include "Math/RandomStream.h"

#define LOCTEXT_NAMESPACE "PCGPlaneSamplerElement"


FPCGElementPtr UPCGPlaneSamplerSettings::CreateElement() const
{
	return MakeShared<FPCGPlaneSamplerElement>();
}

bool FPCGPlaneSamplerElement::ExecuteInternal(FPCGContext* Context) const// cannot change the name of 'Context'
{
	const UPCGPlaneSamplerSettings* settings = Context->GetInputSettings<UPCGPlaneSamplerSettings>();
	if (!settings) return true;

	UPCGPointData*	pn_data = NewObject<UPCGPointData>();
	UPCGMetadata*	mt_data = pn_data->Metadata;
	mt_data->Initialize(nullptr);
	pn_data->InitializeFromData(nullptr); // 入力元がない場合は nullptr でOK

	FBox bounds = FBox(EForceInit::ForceInit);
	for (const FPCGTaggedData& TaggedData : Context->InputData.TaggedData){
		if (const UPCGSpatialData* sp_data = Cast<UPCGSpatialData>(TaggedData.Data)){
			bounds += sp_data->GetBounds();
		}
	}

	const FVector&	min = bounds.Min;
	const FVector&	max = bounds.Max;
	const float		step_x = settings->mVoxelSize.X;
	const float		step_y = settings->mVoxelSize.Y;
	const FVector	b_min(-settings->mObjectSize.X*0.5f, -settings->mObjectSize.Y*0.5f, 0 );
	const FVector	b_max( settings->mObjectSize.X*0.5f,  settings->mObjectSize.Y*0.5f, settings->mObjectSize.Z );

	FRandomStream stream( settings->mSeed );


	TArray<FPCGPoint> points;
	for (float x = min.X; x <= max.X; x += step_x){
		for (float y = min.Y; y <= max.Y; y += step_y){
			const FVector pos(x, y, (min.Z+max.Z)*0.5f + settings->mPlaneZ );
			FPCGPoint p;
			p.Transform = FTransform(pos); // Z = 0 平面
			p.Density   = 1.0f;//stream.FRand(); // 0?1のランダム値
			p.Seed		= stream.RandHelper(INT32_MAX); // ←これ大事！
			p.MetadataEntry = mt_data->AddEntry(); // ←これも必須！
			p.BoundsMin = b_min ;
			p.BoundsMax = b_max ;
			points.Add(p);
		}
	}
	pn_data->GetMutablePoints() = MoveTemp(points);

	FPCGTaggedData& output = Context->OutputData.TaggedData.Emplace_GetRef();
	output.Data = pn_data;

	return true;
}

#undef LOCTEXT_NAMESPACE
