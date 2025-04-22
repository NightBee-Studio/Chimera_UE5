// Fill out your copyright notice in the Description page of Project Settings.


#include "TsLocalizeService.h"

#include "System/TsSystem.h"

#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Internationalization/TextFormatter.h"
#include "Internationalization/ITextFormatArgumentModifier.h"
#include "Internationalization/Regex.h"

#include "Game/Services/Item/TsItemService.h"




//static int EnumToInt(UEnum* en, FName enum_name)
//{
//	if ( en ) return en->GetValueByName(enum_name);
//	return INDEX_NONE ;
//}


// -------------------------------------------------------------------------------------------------------
// FTsRule
//   Rule for Localize
// 

struct  FTsRule
{
public:
	bool GetMatches( TArray<FString>&		results	,
					int						num		,
					const FRegexPattern &	pattern	,
					const FString&			token ) const {	//FRegexPattern(TEXT("([^.]+)\\.([^\\}]+)"))
		FRegexMatcher matcher(pattern, token);// {Name:Variable}の部分
		if (matcher.FindNext()) {
			for ( int i=0 ; i<num ; i++  ){
				results.Add( matcher.GetCaptureGroup(i+1) );
			}
			return true;
		}
		return false;
	}

	virtual bool	Format(FText& text, const FString& token) const {
		return false;
	}
};

struct  FTsVariableRule
	: public FTsRule
{
public:
	virtual bool	Format(FText& text, const FString& token) const override {
		TArray<FString> vars ;
		if ( GetMatches(vars, 2, FRegexPattern(TEXT("([^:]+):([^\\}]+)")), token)) {
			if (UTsParams* param = TsSERVICE(UTsParamService)->GetParams(FName(*vars[0])))
				if (FTsParam* variable = param->GetParam(FName(*vars[1]))) {
					FString value = variable->GetString();
					UE_LOG(LogTemp, Log, TEXT("Format(%s) => (%s)"), *token, *value);
					text = FText::Format(text, { FFormatArgumentValue(FText::FromString(value)) });
					return true;
				}
		}
		return false;
	}
};


struct  FTsArrayIndexRule
	: public FTsRule
{
public:
	virtual bool	Format(FText& text, const FString& token) const override {
		TArray<FString> vars;
		if (GetMatches(vars, 3, FRegexPattern(TEXT("([^:]+):([^\\}]+)\\[([0-9]+)\\]")), token)) {
			int		index = FCString::Atoi(*vars[2]);
			UTsParams* param = TsSERVICE(UTsParamService)->GetParams(FName(*vars[0]));
			if ( param )
				if (FTsParam* variable = param->GetParam(FName(*vars[1]))) {
					FTsParam* value = variable->GetParam(index);
					UE_LOG(LogTemp, Log, TEXT("Format(%s) => (%s)"), *token, *value->GetString());
					text = FText::Format(text, { FFormatArgumentValue(FText::FromString(value->GetString())), });
					return true;
				}
		}
		return false;
	}
};




// -------------------------------------------------------------------------------------------------------
// Implement work
// 
// 

class TsLocalizeServiceImpl {
public:
	TArray<FTsRule*>		mRules;
	TObjectPtr<UDataTable>	mTextData;

	TsLocalizeServiceImpl(TObjectPtr<UTsLocalizeService> owner) {
		mRules = {
			new FTsArrayIndexRule()	,
			new FTsVariableRule()	,
		} ;
	}

	~TsLocalizeServiceImpl(){
		for ( auto r : mRules ){
			delete r ;
		}
	}
};


// -------------------------------------------------------------------------------------------------------
// UTsLocalizeService  Methods
// 
// 

FText UTsLocalizeService::GetText(const FName& region_key)
{
	TsLocalizeServiceImpl* work = static_cast<TsLocalizeServiceImpl*>(mImplWork);

	if (FTsLocalize* loc = work->mTextData->FindRow<FTsLocalize>(region_key, TEXT(""))) {
		FText result = loc->mText;

		//正規表現パターンを定義: {????}などのtokenをキャプチャする
		FRegexPattern token_pattern(TEXT("\\{([^\\}]+)\\}"));
		FRegexMatcher token_matcher(token_pattern, result.ToString());

		// 正規表現マッチを順に探して、Tokenを抽出
		while (token_matcher.FindNext()) {
			FString token = token_matcher.GetCaptureGroup(1);		// キャプチャグループ1が {??????}の部分

			bool done = false;
			for (auto* rule : work->mRules) {						// tokenに対して各ルールを適用する
				if ((done = rule->Format(result, token))) {
					break;
				}
			}
			if (!done) {
				UE_LOG(LogTemp, Log, TEXT(" Token[%s] is not delevered."), *token);
			}
		}
		//UE_LOG(LogTemp, Log, TEXT("GetText(%s)=>%s"), *region_key.ToString(), *result.ToString());
		return result;
	}
	return FText::GetEmpty();
}




// -------------------------------------------------------------------------------------------------------
// UTsLocalizeService
// 
// 

UTsLocalizeService::UTsLocalizeService() 
	: mImplWork(new TsLocalizeServiceImpl(this))
	, mEnumFormats({ StaticEnum<EItemType>() }) 
{
}


void	UTsLocalizeService::Initialize(USubsystem* owner, FSubsystemCollectionBase& collection)
{
	TsLocalizeServiceImpl* work = static_cast<TsLocalizeServiceImpl*>(mImplWork);

	Super::Initialize(owner, collection);


	UTsCocoaSpecs* ddd = NewObject<UTsCocoaSpecs>();
	ddd->mSpecs.Add({ 350,810 });
	ddd->mSpecs.Add({ 250,820 });
	ddd->mSpecs.Add({ 150,830 });
	ddd->mSpecs.Add({ 100,840 });
	ddd->mGold = 300;
	ddd->mGgg = { .mRecipie=10, .mGold = 600 };
	ddd->mPos = FVector(22, 33, 44);
	ddd->mRot = FVector2D(888, 999);

	if (UTsParams* param = TsSERVICE(UTsParamService)->GetParams(FName("TsCocoaSpecs_0")))
		if (FTsParam* variable = param->GetParam(FName("Ggg"))) {
			FTsParam* recipie = variable->GetParam(FName("Recipie"));
			UE_LOG(LogTemp, Log, TEXT("CoCoa  [%s] addr= %p  reci=[%s]"), *variable->GetString(), variable->mContainer, *recipie->GetString());
		}

	FString lang = FInternationalization::Get().GetCurrentCulture()->GetName();
	if (mResourcePaths.Contains(lang)) {// 文字リソースをロードする
		work->mTextData = Cast<UDataTable>(mResourcePaths[lang].TryLoad());
	}

	FText t = GetText(FName(TEXT("UI.Button.Select")));
	UE_LOG(LogTemp, Log, TEXT("UTsLocalizeService =%s"), *t.ToString());
}

void	UTsLocalizeService::Deinitialize() 
{
	Super::Deinitialize();
}
