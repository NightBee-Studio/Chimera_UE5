// Fill out your copyright notice in the Description page of Project Settings.


#include "TsItemService.h"
//#include "ItemFramework/Actor.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/DataTable.h"


class TsItemServiceImpl {
	friend class UTsItemService;
private:

public:
	TsItemServiceImpl(TObjectPtr<UTsItemService> owner){

		//const UDataTable* table = Cast<UDataTable>(DiffArgs.OldAsset);
		//if (table != NULL){
		//	FItemSpec* item = table->FindRow<FItemSpec>( FName("Name"), "" ) ;
		//}

		UE_LOG(LogTemp, Log, TEXT("UTsItemService mSpecs ..."));

		//TSharedPtr<FStreamableHandle> handle = 
		//	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		//		FSoftObjectPath("/Resources/Items/Configs/ItemSpecs"),
		//		[this, handle]() {
		//			mSpecs = Cast<UDataTable>(handle->GetLoadedAsset());
		//			UE_LOG(LogTemp, Log, TEXT("UTsItemService mSpecs completed"));
		//		}
		//	);
	}
};


UTsItemService::UTsItemService()
	//: Super(EAppServiceFlag::E_PrePhysics )
{
}

void UTsItemService::Initialize( USubsystem* owner, FSubsystemCollectionBase& col )
{
	Super::Initialize(owner, col);

	//handle->ReleaseHandle();
	mImplWork = new TsItemServiceImpl( this ) ;
	UE_LOG(LogTemp, Log, TEXT("UTsItemService Initialize") );
}

void UTsItemService::Deinitialize()
{
	if (mImplWork != nullptr) {
		delete mImplWork;
		mImplWork = nullptr;
	}
	UE_LOG(LogTemp, Log, TEXT("UTsItemService Deinitialize"));
}


//void UTsItemService::Update(EAppServiceFlag f )
//{
//}



#if 0


// --------------------------------------------------------------
// Find
public
Item		UTsItemService::Find(ItemType ty, ItemStatus st = ItemStatus.S_None) {
	TsItemServiceImpl* work = (TsItemServiceImpl*)mImplWork;
	return work->mItems
		System.Array.Find(
		gItems,
		a = > a != null &&
		(ty > 0 ? a.mType == ty : true) &&
		(st > 0 ? a.IsStatus(st) : true)
	);
}

public
Item[]	UTsItemService::FindAll(ItemType ty, ItemStatus st = ItemStatus.S_None, ItemFlag fl = ItemFlag.F_None) {
	return System.Array.FindAll(
		gItems,
		a = > a != null &&
		(fl > 0 ? ItemSpec.Get(a.mType).IsFlag(fl) : true) &&
		(ty > 0 ? a.mType == ty : true) &&
		(st > 0 ? a.IsStatus(st) : true)
	);
}

#endif
