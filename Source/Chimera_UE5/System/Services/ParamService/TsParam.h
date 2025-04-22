// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsParam.generated.h"





struct FIdxName {
	FIdxName(int v)
		: mName(*FString::FromInt(v))
	{}

	operator FName() const { return mName; }

private:
	FName					mName;
};


// -------------------------------------------------------------------------------------------------------
// FTsParamSheet
// 
// 

struct FTsParam;
struct FTsParamSheet {
	FTsParamSheet() {}
	FTsParamSheet( UStruct* type, void* container );

	TMap<FName, FTsParam*>	mParamMaps;

	static  FTsParam*		CreateParam(FProperty* prop, void* content);
};




// -------------------------------------------------------------------------------------------------------
// FTsParam
// 
// 

UENUM(BlueprintType)
enum EParamType {					// maybe we don't need this....
	ETpa_None	UMETA(DisplayName = "None"	),
	ETpa_Int	UMETA(DisplayName = "Int"	),
	ETpa_Float	UMETA(DisplayName = "Float"	),
	ETpa_FVec3	UMETA(DisplayName = "Vec3"	),
	ETpa_FVec2	UMETA(DisplayName = "Vec2"	),
	ETpa_Table	UMETA(DisplayName = "Table"	),
	ETpa_Struct	UMETA(DisplayName = "Struct"),
	ETpa_Array	UMETA(DisplayName = "Array"	),
};

struct FTsParam
{
	FTsParam(EParamType ty, FProperty* prop, void* content)
		: mType(ty)					// maybe we don't need this....
		, mProperty(prop)
		, mContainer(content) {}
	virtual ~FTsParam() {}

	EParamType		mType ;			// maybe we don't need this....
	FProperty*		mProperty  ;
	void*			mContainer ;

	virtual FString		GetString()			{ return FString(TEXT("")); }
	virtual FTsParam*	GetParam(FName nm)	{ return nullptr          ; }
	FTsParam*			GetParam(int i)		{ return GetParam(*FString::FromInt(i)); }//array用

	template<typename T>
	T					GetValue()			{ return *(mProperty->ContainerPtrToValuePtr<T>(mContainer)); }
};


// -------------------------------------------------------------------------------------------------------
// UTsParams
// 
// UEEditorやｃ++で定義したUSTRUCT/UCLASSなどのパラメータをプログラムからアクセスできるサービス
// 
// 
// [定義]
// 	USTRUCT(Blueprintable)
//	struct CHIMERA_UE5_API FTsCocoaParam
//	{
//		GENERATED_BODY()
//	public:
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Recipie"))
//		int		mRecipie;
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Gold"))
//		int		mGold;
//	};

//	UCLASS(Blueprintable)
//	class CHIMERA_UE5_API UTsCocoaSpecs
//		: public UTsParams
//	{
//		GENERATED_BODY()
//	public:
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Spec"))
//		TArray<FTsCocoaParam>	mSpecs;
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Pos"))
//		FVector					mPos;
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Rot"))
//		FVector2D				mRot;
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Ggg"))
//		FTsCocoaParam			mGgg;
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Gold"))
//		int						mGold;
//	};
// 
// 
// 
// [生成]
// NewObject<UTsCocoaSpecs>();
// 
// 
// 
// [アクセスの仕方]
//	UTsParams* param = TsSERVICE(UTsParamService)->GetParams( "値のFName" )；
//	if (FTsParam* variable = param->GetParam( "変数名のFName" )) {	
//		int value = variable->GetValue<int>();
//	}
//	if (FTsParam* v_struct = param->GetParam( "変数名のFName" )) {	
//		int value = v_struct->GetParam("メンバ名のFName")->GetValue<int>();
//	}
//	if (FTsParam* v_array = param->GetParam( "変数名のFName" )) {		
//		float value = v_array->GetParam(10)->GetValue<float>();
//	}
// 
// 

UENUM(BlueprintType, Meta = (Bitflags))
enum class EParamPurpose : uint8 {
	ETpm_None		UMETA(DisplayName = "None"		),
	ETpm_SaveData	UMETA(DisplayName = "SaveData"	),
	ETpm_Localize	UMETA(DisplayName = "Localize"	),
	ETpm_Network	UMETA(DisplayName = "Network"	),
};

UCLASS(Blueprintable, Abstract)
class CHIMERA_UE5_API UTsParams
	: public UObject
	, public FTsParamSheet
{
	GENERATED_BODY()
public:
	UTsParams(const FObjectInitializer& init);
	virtual ~UTsParams();

public:
	UPROPERTY(EditAnywhere   , BlueprintReadWrite, Meta = (DisplayName = "Purpose", Bitmask, BitmaskEnum = "EParamPurpose"))
	int32					mPurpose;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Meta = (DisplayName = "ObjectName"))
	FName					mObjectName;

	//UFUNCTION(BlueprintCallable)

	FProperty*				GetProprety(FName name) const;
	FTsParam*				GetParam(FName name)	const;// if array  GetChild( FIdxName(2) )

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int			GetInt( FName name ) const		{ FTsParam* p = GetParam(name); return p ? p->GetValue<int>()   : 0    ; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float		GetFloat( FName name ) const	{ FTsParam* p = GetParam(name); return p ? p->GetValue<float>() : 0.0f ; }
};

