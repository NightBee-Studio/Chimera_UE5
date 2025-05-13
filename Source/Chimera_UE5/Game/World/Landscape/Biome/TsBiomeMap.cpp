
#include "TsBiomeMap.h"

#include "../Util/TsUtility.h"





// -------------------------------- TsMaterialValue  --------------------------------

void	TsMaterialValue::Merge(const TsMaterialValue& a, float rate)
{
	for (auto d : a) {
		Add( TsMaterialPixel(d.mA, d.mAlpha * rate));
	}
}



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
	return range * (v - mMin) / (mMax - mMin);
}

float	TsPlantMap::GetValue(const FVector2D& p)
{
	return TsBiomeMap::GetValue(p);
}

float	TsGenreMap::GetValue(const FVector2D& p) {
	return	FMath::Frac(TsBiomeMap::GetValue(p));
}


TsMountMap::TsMountMap(int w, int h, const FBox2D* bound)
	: TsBiomeMap(w, h, bound, TsNoiseParam()) {
	//SurfaceMountain::gInstance->SetNoiseConfig(TsNoiseParam(1.0f, 0.02f, 0.2f, 0.06f));
}

void	TsMountMap::UpdateRemap(const FVector2D& p)
{
	//SurfaceMountain::gInstance->Exec_UpdateRemap(p);
}

float	TsMountMap::GetValue(const FVector2D& p)
{
	return 0.0f;// SurfaceMountain::gInstance->GetMountValue(p);
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

float		TsHeightMap::RemapImage(float v, float range ) const { 
	return range * (v - mMin)/(mMax-mMin) ;
}

void	TsHeightMap::SaveAll( int x, int y,int w, int h )
{
	Save("TsHeightMap.dds", EImageFile::Dds, EImageFormat::FormatL16_D, x, y, w, h);
}




TsMaterialMap::TsMaterialMap(int w, int h, const FBox2D* bound)
	: TsBiomeMap(w, h, bound, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f))
	, mIndexMap(w, h, bound)
	, mAlphaMap(w, h, bound)
	, mOutIndexMap(w, h, bound)
	, mOutAlphaMap(w, h, bound)
{
}



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
		int idx = list.IndexOfByPredicate([&](const MatScore* s) { return s->mType == ty; });
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


void	TsMaterialMap::SetMaterial(int x, int y, const TsMaterialValue& mv)
{
	auto get_pixel_index = [&](const TsMaterialPixel& px, int i) {
		return	((px.mA & 0xff) << (i * 8));
	};
	auto set_pixel_value = [&](EMaterialType ty, int x, int y, float val) {
		if (!mDic.Contains(ty)) {
			mDic.Add(ty, new TsImageMap<float>(GetW(), GetH(), GetWorld()));
		}
		mDic[ty]->SetPixel(x, y, FMath::Clamp(val, 0.0f, 1.0f));
	};
	auto get_pixel_alpha = [&](const TsMaterialPixel& px, int i, int x, int y) {
		float a = px.mAlpha;
		set_pixel_value(px.mA, x, y, a);
		return ((int)(a * 0xff) << (i * 8));
	};

	TArray<TsMaterialPixel> pix;
	float rate = 0;
	for (int i = mv.Num(); --i >= 0; ) {
		pix.Insert(mv[i], 0);
		if ((rate += mv[i].mAlpha) >= 1.0f) break;
	}
	if (pix.Num() > 4) {// 
		UE_LOG(LogTemp, Log, TEXT("TsMaterialMap:: spillover %d (<4) "), pix.Num());
	}

	int index = 0;
	int alpha = 0;
	for (int i = 0; i < pix.Num(); i++) {
		index |= get_pixel_index(pix[i], i);
		alpha |= get_pixel_alpha(pix[i], i, x, y);
	}

	// Material-Map Pixel-Format			Marching-Square Index...
	mAlphaMap.SetPixel(x, y, alpha);
	mIndexMap.SetPixel(x, y, index);
}

void	TsMaterialMap::SaveAll(int x, int y, int w, int h)
{
	UE_LOG(LogTemp, Log, TEXT("TsMaterialMap::SaveAll"));
	mAlphaMap   .Save("mat_value001.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
	mIndexMap   .Save("mat_index001.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
	mOutAlphaMap.Save("mat_value000.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
	mOutIndexMap.Save("mat_index000.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8, x, y, w, h );
}




void	MarchingSquareMap::SaveAll()
{
	float levels[] = { 0.0f, 0.3333f, 0.6667f, 1.0f };

	int file_no = 0;
	for (int i0 = 0; i0 < 4; i0++) {
		for (int i1 = 0; i1 < 4; i1++) {
			for (int i2 = 0; i2 < 4; i2++) {
				for (int i3 = 0; i3 < 4; i3++) {
					mMap.ForeachPixel([&](int px, int py) {
							float dx = (float)px / mMap.GetW();
							float dy = (float)py / mMap.GetW();
							mMap.SetPixel(	px, py, 
											(levels[i0] * (1 - dx) + levels[i1] * dx)*(1-dy) +
											(levels[i2] * (1 - dx) + levels[i3] * dx)*( dy ) 
								);
						});

					//UE_LOG(LogTemp, Log, TEXT("MarchingSquareMap::<%d> [%d %d %d %d] "), file_no, i0, i1, i2, i3);

					char filename[512];
					sprintf_s(filename, sizeof(filename), "Landscape/Island/Materials/MarchingSquare/march_sq_%03d.dds", file_no++);
					mMap.Save(filename, EImageFile::Dds, EImageFormat::FormatL16);
				}
			}
		}
	}
}
