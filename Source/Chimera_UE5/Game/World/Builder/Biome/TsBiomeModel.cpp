#include "TsBiomeModel.h"

#include "../Util/TsImageMap.h"
#include "../Util/TsUtility.h"


void FTsBiomeModel::Setup()
{
	for (auto& tm : mTextures ) {
		mTextureMaps.Add(tm.Key, new TsTextureMap(tm.Value));
	}
}

void FTsBiomeModel::Lock()
{
	for (auto& tm : mTextureMaps ) tm.Value->Lock();
}

void FTsBiomeModel::UnLock()
{
	for (auto& tm : mTextureMaps ) tm.Value->UnLock();
}

int FTsBiomeModel::GetSizeX()
{
	return mTextureMaps[ETM_Height]->mTex->GetSizeX();
}
int FTsBiomeModel::GetSizeY()
{
	return mTextureMaps[ETM_Height]->mTex->GetSizeY();
}


FTsBiomeModels::FTsBiomeModels( UDataTable* data_table )
{
	if ( !data_table ){
		UE_LOG(LogTemp, Log, TEXT("FTsBiomeModels data_table is null") ) ;
	} else {
		TArray<FTsBiomeSpec*> specs
	;
		data_table->GetAllRows<FTsBiomeSpec>( TEXT("ATsBuilder::Build"), specs );

		for ( FTsBiomeSpec* sp : specs ){
			mSpecs.Add( sp->GetID(), sp );
			UE_LOG(LogTemp, Log, TEXT("BiomeSpec: GType%d SType%d => ID %d  Models=%d"), (int)sp->mGType, (int)sp->mSType, sp->GetID(), sp->mModels.Num());
		}

		for ( auto gn : { EBiomeGType::EBGn_A, EBiomeGType::EBGn_B, EBiomeGType::EBGn_C, EBiomeGType::EBGn_D,}){
			float h = 0 ;
			for ( auto sf : { EBiomeSType::EBSf_Lake, EBiomeSType::EBSf_Field, EBiomeSType::EBSf_Mountain,}){
				int id = FTsBiomeSpec::ID(gn,sf) ;
				if ( mSpecs.Contains( id ) ){
					FTsBiomeSpec* sp = mSpecs[id] ;
					sp->mHeightRange = FVector2D( h, h + sp->mHeight ) ;
					h += sp->mHeight ;
				}
			}
		}

		for ( auto sp : mSpecs ){
			UE_LOG(LogTemp, Log, TEXT("BiomeSpec: GType%d SType%d => ID %d  Models=%d  Range(%f %f)"), (int)sp.Value->mGType, (int)sp.Value->mSType, sp.Value->GetID(), sp.Value->mModels.Num(), sp.Value->mHeightRange.X, sp.Value->mHeightRange.Y);
		}
	}
}


void FTsBiomeModels::Lock() 
{
	for ( auto sp : mSpecs ){
		for ( auto &m : sp.Value->mModels ){
			m.Setup() ;
			m.Lock() ;
		}
	}
}

void FTsBiomeModels::UnLock()
{
	for ( auto sp : mSpecs ){
		for ( auto &m : sp.Value->mModels ){
			m.UnLock() ;
		}
	}
}

FVector2D		FTsBiomeModels::GetHeightRange( int id ) 
{
	return 	mSpecs[id]->mHeightRange  ;

}

FTsBiomeModel*	FTsBiomeModels::GetRandomModel( int id ) 
{
	TArray<FTsBiomeModel>& models = mSpecs[id]->mModels  ;
//	return &models[ TsUtil::RandRange( 0, models.Num()-1 )] ;
	return &models[0] ;
}

