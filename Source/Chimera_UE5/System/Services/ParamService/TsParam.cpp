// Fill out your copyright notice in the Description page of Project Settings.

#include "TsParam.h"
#include "TsParamService.h"
#include "System/TsSystem.h"


// -------------------------------------------------------------------------------------------------------
// FTsParam
// 
// 

struct FTsParamInt
	: public FTsParam
{
public:
	FTsParamInt(FProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_Int, prop, addr) {}

	virtual FString	GetString() override {
		return FString::FromInt(GetValue<int>());
	}
};

class FTsParamFloat
	: public FTsParam
{
public:
	FTsParamFloat(FProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_Float, prop, addr) {}

	virtual FString	GetString() override {
		return FString::SanitizeFloat(GetValue<float>());
	}
};

class FTsParamStruct
	: public FTsParam
	, public FTsParamSheet
{
public:
	FTsParamStruct(FStructProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_Struct, prop, addr)
		, FTsParamSheet(prop->Struct, prop->ContainerPtrToValuePtr<void>(addr)) {}

	virtual FString		GetString() override {
		FString result = FString("");
		for (auto c : mParamMaps) {
			result += c.Value->GetString();
			result += " ";
		}
		return result;
	}

	virtual FTsParam*	GetParam(FName name) override{
		return mParamMaps.Contains(name) ? mParamMaps[name] : nullptr;
	}
};

class FTsParamArray
	: public FTsParam
	, public FTsParamSheet	//using sheet as a cache... Becoz array can grow, never assign the propery in Constructor, has to be lazy init.
{
public:
	FTsParamArray(FProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_Array, prop, addr)
		, FTsParamSheet(nullptr, nullptr) {}

	virtual FTsParam*	GetParam(FName key) override {	// access like this way, FIName(0)
		if (mParamMaps.Contains(key)) return mParamMaps[key];	//cache check

		int i = FCString::Atoi(*key.ToString());
		//lazy init
		FArrayProperty*		arry = CastField<FArrayProperty>(mProperty);
		FScriptArrayHelper	helper(arry, arry->ContainerPtrToValuePtr<void>(mContainer) );
		FTsParam* param = FTsParamSheet::CreateParam(arry->Inner, helper.GetRawPtr(i));
		mParamMaps.Add(key, param);								//save as cache
		return param;
	}
};



class FTsParamFVec3
	: public FTsParam
{
public:
	FTsParamFVec3(FStructProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_FVec3, prop, addr) {}

	virtual FString		GetString() override {
		FVector v = GetValue<FVector>();
		return FString::Printf(TEXT("%.2f,%.2f,%.2f"), v.X, v.Y, v.Z);
	}
};

class FTsParamFVec2
	: public FTsParam
{
public:
	FTsParamFVec2(FStructProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_FVec2, prop, addr) {}

	virtual FString		GetString() override {
		FVector2D v = GetValue<FVector2D>();
		return FString::Printf(TEXT("%.2f,%.2f"), v.X, v.Y);
	}
};

#if 0
class FTsParamTable
	: public FTsParam
{
	TArray<FTsParam*> mArray;
public:
	FTsParamTable(FProperty* prop, void* addr)
		: FTsParam(EParamType::ETpa_Table, prop, addr) {}

	virtual FString		GetString() override {
		return mArray[i]->GetString();
	}
	virtual FTsParam*	GetParam(FName i) override {
		return mArray[i];
	}
	template<typename T>
	T					GetValue(int i) {
		return GetChild(i)->GetValue<T>();
	}
};
#endif







// -------------------------------------------------------------------------------------------------------
// FTsParamSheet
// 
// 

FTsParamSheet::FTsParamSheet( UStruct* type, void* container)
{
	if (type) {
		UE_LOG(LogTemp, Log, TEXT("---- [%p] FTsParamSheet ----"), (void *)container);
		for (TFieldIterator<FProperty> it(type); it; ++it) {
			FProperty* pr = *it;
			FString name = pr->GetMetaData(TEXT("DisplayName"));
			if (name.IsEmpty()) name = pr->GetName();

			UE_LOG(LogTemp, Log, TEXT("Property: %s (%s)  [%p]"), *name, *pr->GetCPPType(), container);
			mParamMaps.Add(FName(*name), CreateParam(pr, container));
		}
	}
}


FTsParam* FTsParamSheet::CreateParam(FProperty* pr, void* addr)
{
	if (FIntProperty* pp = CastField<FIntProperty   >(pr)) return new FTsParamInt(pr, addr);
	if (FFloatProperty* pp = CastField<FFloatProperty >(pr)) return new FTsParamFloat(pr, addr);
	if (FArrayProperty* pp = CastField<FArrayProperty >(pr)) return new FTsParamArray(pr, addr);
	if (FStructProperty* pp = CastField<FStructProperty>(pr)) {
		if (pp->Struct == TBaseStructure<FVector  >::Get())   return new FTsParamFVec3(pp, addr);
		if (pp->Struct == TBaseStructure<FVector2D>::Get())	  return new FTsParamFVec2(pp, addr);
		//if (pp->Struct == TBaseStructure<UDataTable>::Get())	  return new FTsParamTable(pr, addr);
		return new FTsParamStruct(pp, addr);
	}
	return nullptr;
}




// -------------------------------------------------------------------------------------------------------
// UTsParams
// 
// 

UTsParams::UTsParams(const FObjectInitializer& init)
	: FTsParamSheet(init.GetObj()->GetClass(), init.GetObj())
	, mObjectName(init.GetObj()->GetFName())
{
	UE_LOG(LogTemp, Log, TEXT("---- UTsParams [%s: %p]  ----"), *mObjectName.ToString(), init.GetObj());

	TsSERVICE_Call(UTsParamService, AddParams(mObjectName, this));
}

UTsParams::~UTsParams() {
	UE_LOG(LogTemp, Log, TEXT("---- ~UTsParams [%s]  ---- Removed "), *mObjectName.ToString());

	TsSERVICE_Call(UTsParamService, RemoveParams(mObjectName));
}

FProperty*	UTsParams::GetProprety(FName name) const{
	return mParamMaps.Contains(name) ? mParamMaps[name]->mProperty : nullptr;
}

FTsParam*	UTsParams::GetParam(FName name)	const {
	return mParamMaps.Contains(name) ? mParamMaps[name] : nullptr;
}



// 
//Enum ‚ð int ‚É•ÏŠ·‚·‚é•û–@
// 
//
//UEnum* Enum = StaticEnum<EMyEnum>();
//if (Enum)
//{
//	// Enum–¼‚ðŽw’è‚µ‚Ä int ‚É•ÏŠ·
//	FString EnumName = TEXT("OptionB");
//	int32 EnumValue = Enum->GetValueByName(FName(*EnumName));
//
//	if (EnumValue != INDEX_NONE)
//	{
//		UE_LOG(LogTemp, Log, TEXT("Enum name %s = %d"), *EnumName, EnumValue);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("Invalid enum name: %s"), *EnumName);
//	}
//}
//
//
//
