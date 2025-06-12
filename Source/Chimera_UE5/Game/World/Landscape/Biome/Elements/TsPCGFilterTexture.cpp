

#include "TsPCGFilterTexture.h"

#include "PCGContext.h"
#include "PCGPin.h"
#include "PCGComponent.h"
#include "Data/PCGSpatialData.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"

#include "PCGVolume.h"

//#include UE_INLINE_GENERATED_CPP_BY_NAME(PCGSnapToGround)

#define LOCTEXT_NAMESPACE "PCGFilterTextureElement"


FPCGElementPtr UPCGFilterTextureSettings::CreateElement() const
{
	return MakeShared<FPCGFilterTextureElement>();
}

int FPCGFilterTextureElement::GetIndexFromPos(const FPCGContext* context, const FPCGPoint &point ) const
{
	const UPCGComponent* pcg_comp = context->SourceComponent.Get();
	FVector volpos = pcg_comp->GetOwner()->GetActorLocation();
	FVector scale  = pcg_comp->GetOwner()->GetActorScale3D();
	FVector volsiz(100.0f);

	// ComponentがPCGVolumeに接続されている前提でキャスト
	if (const APCGVolume* pcg_vol = Cast<APCGVolume>(pcg_comp->GetOwner())){
		volsiz = pcg_vol->GetBounds().BoxExtent * 2.0f;
	}

	const UPCGFilterTextureSettings* settings = context->GetInputSettings<UPCGFilterTextureSettings>();
	check(settings);
	const UTexture2D* tex = settings->mTexture;

	int byte_per_pixel = GPixelFormats[tex->GetPixelFormat()].BlockBytes;
	int w = tex->GetSizeX();
	int h = tex->GetSizeY();

	FVector local = (point.Transform.GetLocation() + volsiz * 0.5f - volpos) / volsiz.X * w ;
	int x = FMath::Clamp( (int)(local.X), 0, w-1) ;
	int y = FMath::Clamp( (int)(local.Y), 0, h-1);
	//UE_LOG(LogTemp, Log, TEXT("VolSize %f %f %f ) * scale %f x%d y%d ofs%d"), volsiz.X, volsiz.Y, volsiz.Z, scale.X,
	//	x,y, (y * w + x) * byte_per_pixel);
	return (y*w + x) * byte_per_pixel;
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

		const UPCGFilterTextureSettings* settings = Context->GetInputSettings<UPCGFilterTextureSettings>();
		check(settings);
		float	    min = settings->mRange.X;
		float 	    max = settings->mRange.Y;
		UTexture2D* tex = settings->mTexture;

#if WITH_EDITORONLY_DATA
		if (tex && tex->Source.IsValid()){
			//UE_LOG(LogTemp, Log, TEXT("Source  IsValid%d Fmt%d"), tex ? tex->Source.IsValid() : -2, tex->GetPixelFormat());
#else
		if (tex && tex->GetPlatformData() && tex->GetPlatformData()->Mips.Num()>0 ){
			//UE_LOG(LogTemp, Log, TEXT("Platform MipNum%d"), tex ? tex->GetPlatformData() ? tex->GetPlatformData()->Mips.Num() : -2 : -1);
#endif
			const TArray<FPCGPoint>& points = pn_data->GetPoints();
			UPCGPointData* out_data = NewObject<UPCGPointData>();
			out_data->InitializeFromData(pn_data);
			TArray<FPCGPoint>& out_points = out_data->GetMutablePoints();
			output.Data = out_data;

#if WITH_EDITORONLY_DATA
			const void* data_ptr = (const void*)tex->Source.LockMip(0);
#else
			FTexture2DMipMap& mip = tex->GetPlatformData()->Mips[0];
			const void* data_ptr = mip.BulkData.LockReadOnly();
#endif
			for (const FPCGPoint& p : points) {
				int    idx = GetIndexFromPos(Context, p);
				float  r   = 0.0f;
				switch( tex->GetPixelFormat() ){
				case EPixelFormat::PF_G16:
				case EPixelFormat::PF_R16_UINT:
				case EPixelFormat::PF_R16_SINT:
					r = ((uint16*)data_ptr)[idx/2] / 65535.0f;
					break;
				case EPixelFormat::PF_B8G8R8A8:
				case EPixelFormat::PF_R8G8B8A8:
					r = ((uint8 *)data_ptr)[idx+0] / 255.0f   ;
					break;
				}
				if (min <= r && r <= max) {
					//UE_LOG(LogTemp, Log, TEXT("(pix%d  r%f fmt%d) "), ((uint16*)data_ptr)[idx / 2], r, tex->GetPixelFormat());
					out_points.Add(p);
				}
			}
#if WITH_EDITORONLY_DATA
			tex->Source.UnlockMip(0);
#else
			mip.BulkData.Unlock();
#endif
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE

