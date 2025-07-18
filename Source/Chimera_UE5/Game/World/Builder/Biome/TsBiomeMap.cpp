#include "TsBiomeMap.h"

#include "../Util/TsImageMap.h"
#include "../Util/TsUtility.h"



TMap<EBiomeMapType, TsBiomeMap*>	TsBiomeMap::gBiomeMaps;


// -------------------------------- TsBiomeMap  --------------------------------
//( 1.0f, 0.0010f, 0.2f, 0.0030f )

float	TsBiomeMap::GetValue(const FVector2D& p)	// world-coord
{
	float v = 0;
	if ( mLayers.Num() == 0 ) {
		FIntVector2	px = GetPixelPos(p);
		v = TsImageMap<float>::GetPixel(px.X, px.Y);	// get pixel value
	} else {
		v = TsNoiseMap::GetValue(p);					// calc noise value
		TsImageMap<float>::SetPixel(p, v);
	}
	mMin = FMath::Min(mMin, v);// update ing the remap
	mMax = FMath::Max(mMax, v);
	return v;
}

void	TsBiomeMap::SetPixel(int x, int y, float v)
{
	mMin = FMath::Min(mMin, v);// update ing the remap
	mMax = FMath::Max(mMax, v);
	TsImageMap<float>::SetPixel(x, y, v);
}

float	TsBiomeMap::RemapImage(float v, float range) const {
	return (mMax - mMin) > 0 ? range * (v - mMin) / (mMax - mMin) : v;
}



//calc normal
// 
// Triangle(h01,h10,h00)
// Triangle(h10,h01,h11)
// // 
//  h00---h01
//   |  /  |
//  h10---h11
// 
// choose which triangle will be inclued in (x,y) 
//

FVector TsHeightMap::CalcNormal(float x, float y, float grid_height, float grid_size) {
	auto get_height = [&](int x, int y) { return GetPixel(x, y) * grid_height; };

#if 0
	glm::vec3 n;
	n  = glm::vec3(0.15) * glm::normalize(glm::vec3(scale * (TsHeightMap[i][j] - TsHeightMap[i + 1][j]), 1.0, 0.0));  //Positive X
	n += glm::vec3(0.15) * glm::normalize(glm::vec3(scale * (TsHeightMap[i - 1][j] - TsHeightMap[i][j]), 1.0, 0.0));  //Negative X
	n += glm::vec3(0.15) * glm::normalize(glm::vec3(0.0, 1.0, scale * (TsHeightMap[i][j] - TsHeightMap[i][j + 1])));    //Positive Y
	n += glm::vec3(0.15) * glm::normalize(glm::vec3(0.0, 1.0, scale * (TsHeightMap[i][j - 1] - TsHeightMap[i][j])));  //Negative Y

	//Diagonals! (This removes the last spatial artifacts)
	n += glm::vec3(0.1) * glm::normalize(glm::vec3(scale * (TsHeightMap[i][j] - TsHeightMap[i + 1][j + 1]) / sqrt(2), sqrt(2), scale * (TsHeightMap[i][j] - TsHeightMap[i + 1][j + 1]) / sqrt(2)));    //Positive Y
	n += glm::vec3(0.1) * glm::normalize(glm::vec3(scale * (TsHeightMap[i][j] - TsHeightMap[i + 1][j - 1]) / sqrt(2), sqrt(2), scale * (TsHeightMap[i][j] - TsHeightMap[i + 1][j - 1]) / sqrt(2)));    //Positive Y
	n += glm::vec3(0.1) * glm::normalize(glm::vec3(scale * (TsHeightMap[i][j] - TsHeightMap[i - 1][j + 1]) / sqrt(2), sqrt(2), scale * (TsHeightMap[i][j] - TsHeightMap[i - 1][j + 1]) / sqrt(2)));    //Positive Y
	n += glm::vec3(0.1) * glm::normalize(glm::vec3(scale * (TsHeightMap[i][j] - TsHeightMap[i - 1][j - 1]) / sqrt(2), sqrt(2), scale * (TsHeightMap[i][j] - TsHeightMap[i - 1][j - 1]) / sqrt(2)));    //Positive Y


	float h00 = get_height(x + 0, y + 0);
	float h01 = get_height(x + 0, y + 1);
	float h10 = get_height(x + 1, y + 0);
	float h11 = get_height(x + 1, y + 1);
#define T   grid_size
	if (FMath::Frac(x) < FMath::Frac(y)) {
		return FVector::CrossProduct(
			FVector( 0, h00 - h01, T).GetSafeNormal(),
			FVector(-T, h11 - h01, 0).GetSafeNormal()).GetSafeNormal();
	}
	else {
		return FVector::CrossProduct(
			FVector(-T, h00 - h10, 0).GetSafeNormal(),
			FVector( 0, h11 - h10, T).GetSafeNormal()).GetSafeNormal();
	}
#else
	/*
	  Note: Surface normal is computed in this way, because the square-grid surface is meshed using triangles.
	  To avoid spatial artifacts, you need to weight properly with all neighbors.
	*/
	float   h_00 = get_height(x  , y  );
	float   h_p0 = get_height(x+1, y  ), h_m0 = get_height(x-1, y  );
	float   h_0p = get_height(x  , y+1), h_0m = get_height(x  , y-1);
	float   h_pp = get_height(x+1, y+1), h_pm = get_height(x+1, y-1);
	float   h_mp = get_height(x-1, y+1), h_mm = get_height(x-1, y-1);

	FVector n;
	n  = 0.15f*(  FVector(h_00-h_p0, 1, 0        ).GetSafeNormal()
				+ FVector(h_m0-h_00, 1, 0        ).GetSafeNormal()
				+ FVector(0        , 1, h_00-h_0p).GetSafeNormal()
				+ FVector(0        , 1, h_0m-h_00).GetSafeNormal() );

	//Diagonals! (This removes the last spatial artifacts)
	n += 0.10f*(  FVector(h_00-h_pp, 2, h_00-h_pp).GetSafeNormal()
				+ FVector(h_00-h_pm, 2, h_00-h_pm).GetSafeNormal()
				+ FVector(h_00-h_mp, 2, h_00-h_mp).GetSafeNormal()
				+ FVector(h_00-h_mm, 2, h_00-h_mm).GetSafeNormal() );
	return n;
#endif
}

float	TsHeightMap::RemapImage(float v, float range ) const { 
	return range * (mMax-mMin>0 ? FMath::Clamp( (v - mMin)/(mMax-mMin), 0, 1 ) : v) ;
}

void	TsHeightMap::Normalize()
{
	ForeachPixel(
		[&](int px, int py) {
			SetPixel(px, py, TsValueMap::Remap(GetPixel(px, py)));
		});
	RemapDone();
}




TsMaterialMap::TsMaterialMap(int w, int h, const FBox2D* bound, const TArray<EMaterialType>& mat_index)
	: TsImageCore(w, h, 1, FString() ) 
	, mPixels()
	, mIndexMap(w, h, bound)
	, mAlphaMap(w, h, bound)
	, mMatIndex(mat_index)
{
	if (bound != nullptr) {
		SetWorld(bound);
	}
	mPixels.Init(TsMaterialPixel(), w * h);
}

void	TsMaterialMap::SetPixel(int x, int y, EMaterialType ty, float val) 
{
	mPixels[x + mW * y].Add(ty, val);
}

TsMaterialPixel& TsMaterialMap::GetPixel(int x, int y) 
{
	return mPixels[x + y * mW];
}

void	TsMaterialMap::MergePixel(int x, int y, const TsMaterialPixel& px)
{
	GetPixel(x, y).Merge( px );
	GetPixel(x, y).Normalize();
}

void	TsMaterialMap::ForeachPix(std::function< void(int, int, TsMaterialPixel&) > func, int inc )
{
	for (int y = 0; y < mH; y += inc) {
		for (int x = 0; x < mW; x += inc) {
			func(x, y, mPixels[x + y*mW] );
		}
	}
}

void	TsMaterialMap::StoreMaterial()
{
	auto get_pixel_index = [&](int t, int i) {
		return	((t & 0xff) << (i * 8));
	};
	auto get_pixel_alpha = [&](float v, int i) {
		return ((int)(v * 0xff) << (i * 8));
	};

	// Material-Map Pixel-Format			Marching-Square Index...
	ForeachPixel(
		[&](int px, int py) {
			TsMaterialPixel& pix = GetPixel(px, py);
			int index = 0;
			int alpha = 0;
			int i = 0;
			for ( auto & p : pix.mValues ) {
				//UE_LOG(LogTemp, Log, TEXT(" [%d,%d]  %d => %d"),   px,py,  p.Key, mMatIndex.Find(p.Key) );
				index |= get_pixel_index(mMatIndex.Find(p.Key), i);
				alpha |= get_pixel_alpha(p.Value, i);
				i++;
			}
			//UE_LOG(LogTemp, Log, TEXT("(%d,%d) %08x %08x"), px, py, index, alpha);
			mAlphaMap.SetPixel(px, py, alpha);
			mIndexMap.SetPixel(px, py, index);
		});
}

void	TsMaterialMap::SaveAll(int x, int y, int w, int h)
{
	TMap<EMaterialType, FString> enumname = {
		{ EBMt_None		,FString("None")		},
		{ EBMt_Soil_A	,FString("Soil_A")		},
		{ EBMt_Soil_B	,FString("Soil_B")		},
		{ EBMt_Soil_C	,FString("Soil_C")		},
		{ EBMt_Grass_A	,FString("Grass_A")		},
		{ EBMt_Grass_B	,FString("Grass_B")		},
		{ EBMt_Forest_A	,FString("Forest_A")	},
		{ EBMt_Forest_B	,FString("Forest_B")	},
		{ EBMt_Rock_A	,FString("Rock_A")		},
		{ EBMt_Moss_A	,FString("Moss_A")		},
		{ EBMt_Moss_B	,FString("Moss_B")		},
	};
	
	int tw = mAlphaMap.GetW();
	int th = mAlphaMap.GetH();
	TsImageMap<float>* bitmap = new TsImageMap<float>( tw, th, mAlphaMap.GetWorld() );
	for (auto& i : mMatIndex) {
		bool need_save = false;
		bitmap->ForeachPixel([&](int x, int y) {
				float val = 0.0f;
				if ( mPixels[x + y * tw].mValues.Contains(i)) {
					val = mPixels[x + y * tw].mValues[i];
					need_save = true;
				}
				bitmap->SetPixel(x, y, val );
			});
		if (need_save){
			FString fname = FString::Printf( TEXT("Mat_%s.dds"), *(enumname[i]) );
			bitmap->Save( *fname, EImageFile::Dds, EImageFormat::FormatR8, 0, 0, w, h);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TsMaterialMap::SaveAll"));
	mAlphaMap   .Save("MatAlpha00.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
	mIndexMap   .Save("MatIndex00.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
}


float	TsMoistureMap::GetValue( const FVector2D& p )
{
	FIntVector2	pos = TsBiomeMap::GetPixelPos( p );
	float		val = TsBiomeMap::GetValue( p ) ;
	for ( auto &ex : mExtras ){
		float pix = ex.mTex->GetPixel( pos.X, pos.Y, EAnchor::E_LB, mW );
		switch( ex.mOp ){
		case EExtraOp::E_Mul:
			val *= (pix    ) * ex.mScale ; 
			break ;
		case EExtraOp::E_InvMul:
			val *= (1 - pix) * ex.mScale ; 
			break ;
		default:
		case EExtraOp::E_Add:
			val += pix * ex.mScale ; 
			break ;
		}
	}
	return val ;
}
