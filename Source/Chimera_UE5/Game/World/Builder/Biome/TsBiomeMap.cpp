
#include "TsBiomeMap.h"

#include "../Util/TsImageMap.h"
#include "../Util/TsUtility.h"







TMap<EBiomeMapType, TsBiomeMap*>	TsBiomeMap::gBiomeMaps;


// -------------------------------- TsBiomeMap  --------------------------------
//( 1.0f, 0.0010f, 0.2f, 0.0030f )

float	TsBiomeMap::GetValue(const FVector2D& p)	// world-coord
{
	float v = 0;
	if (mN0 == 0 && mN1 == 0) {
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
	return range * FMath::Clamp( (v - mMin)/(mMax-mMin), 0, 1 ) ;
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


#if 0
void	TsMaterialMap::SetMaterialPixel(int x, int y)
{
	struct MatScore {
		float	mScore;
		int		mScoreNum;
		int		mType;
		int		mAlpha;

		bool operator < (const MatScore& a) const { return mScore / mScoreNum > a.mScore / a.mScoreNum; }// sorting be random value

		MatScore(int ty) : mScore(0), mScoreNum(0), mType(ty), mAlpha(0) {}

		void AddAlpha(int i, int alpha, int score) {
			mAlpha |= ((alpha > 220 ? 3 :
						alpha > 135 ? 2 :
						alpha >  50 ? 1 : 0) << (i * 2));
			mScore += score;
			mScoreNum++;
		}
	};

	auto get = [&](int data, int i) { return ((data >> (i * 8)) & 0xff); };
	auto set = [&](int data, int i) { return ((data & 0xff) << (i * 8)); };

	TArray<MatScore*>	list;
	auto get_list = [&](int ty) {
		int idx = list.IndexOfByPredicate( [&](const MatScore* s) { return s->mType == ty; });
		if ( idx==INDEX_NONE ) {
			idx = list.Num();
			list.Add(new MatScore(ty));
		}
		return list[idx];
	};

	for (int i = 0; i < 4; i++) {		// pixel primeter 
		int px = x + (i&1 ? 1 : 0);
		int py = y + (i&2 ? 1 : 0);
		int alp = mAlphaMap.GetPixel(px, py);
		int idx = mIndexMap.GetPixel(px, py);
		for (int j = 0; j < 4; j++) {	// regist each mat-type
			int ty = get(idx, j);
			int al = get(alp, j);
			if (ty == 0) continue;

			get_list(ty)->AddAlpha(i, al, 3-j);
		}
	}

	list.Sort();

	int index = 0;
	int alpha = 0;
	int i = 0;
	for ( auto m : list) {		// pixel primeter 
		index |= set( m->mType , i );
		alpha |= set( m->mAlpha, i );
		if (i == 0) alpha = 0xff ;

		if (++i == 4) {
			//if ( maps.Num() > 4 )	UE_LOG(LogTemp, Log, TEXT("TsMaterialMap:: SetPixel spilled  %d (<4) "), maps.Num());
			break;
		}
	}

	mOutIndexMap.SetPixel(x, y, index);
	mOutAlphaMap.SetPixel(x, y, alpha);
}
#endif

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
	UE_LOG(LogTemp, Log, TEXT("TsMaterialMap::SaveAll"));
	mAlphaMap   .Save("MatAlpha00.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
	mIndexMap   .Save("MatIndex00.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
}
