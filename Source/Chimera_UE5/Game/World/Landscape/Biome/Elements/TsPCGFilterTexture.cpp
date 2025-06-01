
#include "TsPCGFilterTexture.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPin.h"

#include "Helpers/PCGAsync.h"

#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"


//#include UE_INLINE_GENERATED_CPP_BY_NAME(PCGSnapToGround)

#define LOCTEXT_NAMESPACE "PCGFilterTexture"


TArray<FPCGPinProperties> UPCGFilterTextureSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);

	return PinProperties;
}

TArray<FPCGPinProperties> UPCGFilterTextureSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);

	return PinProperties;
}

FPCGElementPtr UPCGFilterTextureSettings::CreateElement() const
{
	return MakeShared<FPCGFilterTextureElement>(mTexture, mVolumeSize, mThresholdMin, mThresholdMax );
}


FPCGFilterTextureElement::FPCGFilterTextureElement(TObjectPtr<UTexture2D> texture, const FVector& size, float min, float max)
	: IPCGElement()
	, mTexture(texture)
	, mVolumeSize(size)
	, mMin(min)
	, mMax(min<max ? max : min+0.01f)
{
}

int FPCGFilterTextureElement::GetIndexFromPos(const FPCGContext* context, const FPCGPoint &point ) const
{
	const UPCGComponent* pcg_comp = context->SourceComponent.Get();

	FVector volpos = pcg_comp->GetOwner()->GetActorLocation();
	FVector scale  = pcg_comp->GetOwner()->GetActorScale3D();
	FVector volsiz = mVolumeSize * scale.X;// scale

	//if (APCGVolume* pcg_volume = Cast<APCGVolume>(pcg_comp->GetOwner())){
	//	UE_LOG(LogTemp, Log, TEXT("This PCGComponent belongs to a PCGVolume!"));
	//	// ここでサイズ取得可能
	//	UBrushComponent* brush = pcg_volume->GetBrushComponent();
	//	if (brush)	{
	//		FVector box_extent  = brush->Bounds.BoxExtent;
	//		volsiz = brush->GetComponentTransform().TransformVector(box_extent * 2.0f);
	//		UE_LOG(LogTemp, Log, TEXT("PCG Volume Size: %s"), *volsiz.ToString());
	//	}
	//}

	int byte_per_pixel = GPixelFormats[mTexture->GetPixelFormat()].BlockBytes;
	int w = mTexture->GetSizeX();
	int h = mTexture->GetSizeY();

	FVector local = (point.Transform.GetLocation() + volsiz *0.5f - volpos) / volsiz.X * w ;
	int x = FMath::Clamp( (int)(local.X), 0, w-1) ;
	int y = FMath::Clamp( (int)(local.Y), 0, h-1);
	return (x*w + y) * byte_per_pixel;
}

bool FPCGFilterTextureElement::ExecuteInternal(FPCGContext* Context) const// cannot change the name of 'Context'
{
	const TArray<FPCGTaggedData>	inputs	= Context->InputData.GetInputs();
	TArray<FPCGTaggedData>&			outputs	= Context->OutputData.TaggedData;

	for (const FPCGTaggedData& i : inputs){
		TRACE_CPUPROFILER_EVENT_SCOPE( FPCGTransformPointsElement::Execute::InputLoop );
		FPCGTaggedData&		output = outputs.Add_GetRef(i);

		const UPCGSpatialData*	sp_data = Cast<UPCGSpatialData>(i.Data);
		if ( !sp_data ){
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingSpatialData", "Unable to get Spatial data from input"));
			continue;
		}
		const UPCGPointData*	pn_data = sp_data->ToPointData(Context);
		if ( !pn_data ){
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingPointData", "Unable to get Point data from input"));
			continue;
		}

		if (mTexture && mTexture->GetPlatformData() && mTexture->GetPlatformData()->Mips.Num()>0 ){
			const TArray<FPCGPoint>& points = pn_data->GetPoints();
			UPCGPointData* out_data = NewObject<UPCGPointData>();
			out_data->InitializeFromData(pn_data);
			TArray<FPCGPoint>& out_points = out_data->GetMutablePoints();
			output.Data = out_data;

			FTexture2DMipMap& mip = mTexture->GetPlatformData()->Mips[0];
			const void* data_ptr = mip.BulkData.LockReadOnly();
			for (const FPCGPoint& p : points) {
				int    idx = GetIndexFromPos(Context, p);
				float  r   = 0.0f;
				switch( mTexture->GetPixelFormat() ){
				case EPixelFormat::PF_R16_UINT:
				case EPixelFormat::PF_R16_SINT:
					r = ((uint16*)data_ptr)[idx] / 65535.0f;
					break;
				case EPixelFormat::PF_B8G8R8A8:
				case EPixelFormat::PF_R8G8B8A8:
					r = ((uint8 *)data_ptr)[idx] / 255.0f   ;
					break;
				}
				if (mMin <= r && r <= mMax) {
					UE_LOG(LogTemp, Log, TEXT("(r%f fmt%d) "), r, mTexture->GetPixelFormat());
					out_points.Add(p);
				}
			}
			mip.BulkData.Unlock();
		}
	}

	return true;
}
