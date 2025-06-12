#include "TsPCGCalcThreshold.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"

#include "PCGParamData.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Metadata/PCGMetadataAttributeTpl.h"

#include "../TsBiomePCG.h"


#define LOCTEXT_NAMESPACE "TsPCGCalcThreshold"



FPCGElementPtr UPCGCalcThresholdSettings::CreateElement() const
{
	return MakeShared<FPCGCalcThresholdElement>();
}

bool FPCGCalcThresholdElement::ExecuteInternal(FPCGContext* Context) const
{
	const TArray<FPCGTaggedData>& inputs  = Context->InputData .TaggedData;
	      TArray<FPCGTaggedData>& outputs = Context->OutputData.TaggedData ;

#if 1
	const UPCGCalcThresholdSettings* settings = Context->GetInputSettings<UPCGCalcThresholdSettings>();
	check(settings);
	UTexture2D* tex = settings->mTexture;

	TArray<float> samples;

	{// calculate from texture...
#if WITH_EDITORONLY_DATA
		const void* data_ptr = (const void*)tex->Source.LockMip(0);
#else
		FTexture2DMipMap& mip = tex->GetPlatformData()->Mips[0];
		const void* data_ptr = mip.BulkData.LockReadOnly();
#endif
		int w = tex->GetSizeX();
		int h = tex->GetSizeY();
		for (int x = 0; x < w; x += 2) {
			for (int y = 0; y < h; y += 2) {
				int idx = (y * w + x);
				switch (tex->GetPixelFormat()) {
				case EPixelFormat::PF_G16:
				case EPixelFormat::PF_R16_UINT:
				case EPixelFormat::PF_R16_SINT:
					samples.Add((float)((uint16*)data_ptr)[idx] / 65535.0f);
					break;
				case EPixelFormat::PF_B8G8R8A8:
				case EPixelFormat::PF_R8G8B8A8:
					samples.Add((float)((uint8*)data_ptr)[idx * 4] / 255.0f);
					break;
				}
			}
		}
#if WITH_EDITORONLY_DATA
		tex->Source.UnlockMip(0);
#else
		mip.BulkData.Unlock();
#endif
	}
	// sort the samaples to decide the threshold.
	samples.Sort();

	// threshold from index amount..
	TArray<float>	results;
	float ratio = 0;
	int   max = (samples.Num() - 1);
	for (auto& ly : settings->mLayers) {
		ratio += ly.mRatio;
		results.Add( samples[max * FMath::Min(1, ratio)] );
	}
#endif

	UPCGPointData* pn_data = NewObject<UPCGPointData>();
	pn_data->InitializeFromData(nullptr);

	//UPCGParamData* out_data = NewObject<UPCGParamData>();
	pn_data->Metadata->CreateAttribute<float>(TEXT("R1"), results[0], false, false);
	pn_data->Metadata->CreateAttribute<float>(TEXT("R2"), results[1], false, false);
	pn_data->Metadata->CreateAttribute<float>(TEXT("R3"), results[2], false, false);
	pn_data->Metadata->CreateAttribute<float>(TEXT("R4"), results[3], false, false);
	pn_data->Metadata->AddEntry();

	//FPCGPoint point;
	//point.Transform = FTransform::Identity;
	pn_data->GetMutablePoints().Add( FPCGPoint() );

	FPCGTaggedData td;
	td.Data = pn_data;
	td.Pin  = TEXT("Out");
	outputs.Add(MoveTemp(td));

	return true;
}


#undef LOCTEXT_NAMESPACE
