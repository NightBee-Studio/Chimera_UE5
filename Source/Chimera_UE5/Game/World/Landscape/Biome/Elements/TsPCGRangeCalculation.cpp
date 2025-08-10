#include "TsPCGRangeCalculation.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"

#include "PCGParamData.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Metadata/PCGMetadataAttributeTpl.h"

#include "../TsBiomePCG.h"


#define LOCTEXT_NAMESPACE "TsPCGRangeCalculation"



FPCGElementPtr UPCGRangeCalculationSettings::CreateElement() const
{
	return MakeShared<FPCGRangeCalculationElement>();
}

bool FPCGRangeCalculationElement::ExecuteInternal(FPCGContext* Context) const
{
	const TArray<FPCGTaggedData>& inputs  = Context->InputData .TaggedData;
	      TArray<FPCGTaggedData>& outputs = Context->OutputData.TaggedData ;

	const UPCGRangeCalculationSettings* settings = Context->GetInputSettings<UPCGRangeCalculationSettings>();
	check(settings);
	UTexture2D* tex = settings->mTexture;
	if ( !tex ) 	return true;

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
				int fmt = tex->GetPixelFormat();
				float value = 0 ;
				switch (fmt) {
				case EPixelFormat::PF_G16:
				case EPixelFormat::PF_R16_UINT:
				case EPixelFormat::PF_R16_SINT:
					value = (float)((uint16*)data_ptr)[idx] / 65535.0f;
					break;
				case EPixelFormat::PF_B8G8R8A8:
				case EPixelFormat::PF_R8G8B8A8:
					value = (float)((uint8*)data_ptr)[idx * 4] / 255.0f ;
					break;
				case EPixelFormat::PF_G8:
					value = (float)((uint8*)data_ptr)[idx]     / 255.0f ;
					break;
				default:
					break;
				}
				if ( value > 0 ) samples.Add( value ) ;
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

	UPCGPointData* pn_data = NewObject<UPCGPointData>();
	pn_data->InitializeFromData(nullptr);

	pn_data->Metadata->CreateAttribute<float>(TEXT("R1"), results.Num()>0 ? results[0] : 0.0f, false, false);
	pn_data->Metadata->CreateAttribute<float>(TEXT("R2"), results.Num()>1 ? results[1] : 0.0f, false, false);
	pn_data->Metadata->CreateAttribute<float>(TEXT("R3"), results.Num()>2 ? results[2] : 0.0f, false, false);
	pn_data->Metadata->CreateAttribute<float>(TEXT("R4"), results.Num()>3 ? results[3] : 0.0f, false, false);
	pn_data->Metadata->AddEntry();
	pn_data->GetMutablePoints().Add( FPCGPoint() );

	FPCGTaggedData td = { .Data = pn_data, .Pin = TEXT("Out"),};
	outputs.Add(MoveTemp(td));

	return true;
}


#undef LOCTEXT_NAMESPACE
