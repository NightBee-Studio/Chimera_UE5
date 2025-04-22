// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../TsSystem.h"
#include "TsLocalizeService.generated.h"

//
// SaveData for local-save
// Net Transfer for DB.
//


USTRUCT()
struct CHIMERA_UE5_API FTsLocalize
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Data", meta = (DisplayName = "Text"))
	FText				mText;
};


UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsLocalizeService
	: public UTsService
{
	GENERATED_BODY()

private:
	// Implement
	void*						mImplWork;


public:
	UTsLocalizeService();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (DisplayName = "LocalizedData"))
	TMap<FString, FSoftObjectPath>	mResourcePaths;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (DisplayName = "EnumFormats"))
	TArray<UEnum*>					mEnumFormats;

	FText			GetText( const FName& key );

	virtual void	Initialize( USubsystem* owner, FSubsystemCollectionBase& collection) override;
	virtual void	Deinitialize() override;

	virtual void	Update( EServiceFlag f ) override {}
};















USTRUCT(Blueprintable)
struct CHIMERA_UE5_API FTsCocoaParam
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Recipie"))
	int		mRecipie;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Gold"))
	int		mGold;
};

UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsCocoaSpecs
	: public UTsParams
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Spec"))
	TArray<FTsCocoaParam> mSpecs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Pos"))
	FVector		mPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Rot"))
	FVector2D	mRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Ggg"))
	FTsCocoaParam	mGgg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Gold"))
	int		mGold;
};





// LocalizeService
// 
//	https://dev.epicgames.com/documentation/ja-jp/unreal-engine/text-localization-in-unreal-engine?application_version=5.2
//

//ローカライズ
// 
// 
// ローカライズ済みのテキストをデータテーブルで管理すると、大規模なプロジェクトで便利
// データテーブルを FText 型にしておけば、エディタからも翻訳可能
//		1.FTextTableRow 構造体を作成
//		2.データテーブルアセットを作成して FTextTableRow を適用
//		3.LOCTEXT でキーを生成
// 
//	USTRUCT(BlueprintType)
//	struct FTextTableRow : public FTableRowBase
//	{
//		GENERATED_BODY()
//
//	public:
//		UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		FText	mText;
//	};
//
// 
//	LOCTEXT で定義したテキストを Blueprint で利用可能
//	UPROPERTY で FText 型を定義すると、エディタで直接編集可能
// 
//	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/LocalizedTextTable.LocalizedTextTable"));
//	if (DataTable){
//		FTextTableRow* Row = DataTable->FindRow<FTextTableRow>(FName("MyTextKey"), TEXT(""));
//		if (Row){
//			FText MyText = Row->LocalizedText;
//			UE_LOG(LogTemp, Log, TEXT("%s"), *MyText.ToString());
//		}
//	}
//
//	FText::Format() で文字列に動的な値を挿入できる
//
// 
// 
// おすすめ戦略
//	 ✅ 通常のローカライズ → LOCTEXT
//	 ✅ グローバルなキー管理 → NSLOCTEXT
//	 ✅ 動的な値 → FText::Format()
//	 ✅ 大量のテキスト → データテーブル
//	 ✅ Blueprint で利用 → UPROPERTY で FText 定義
// 
// 



// テキストのフォーマッティング
//	ITextFormatArgumentModifier 
//	FTextFormatter::RegisterTextArgumentModifier 
//
//"There {NumCats}|plural(one=is,other=are) {NumCats} {NumCats}|plural(one=cat,other=cats)"
//
//Tool->LocalizationDashboard
//



//void FMyGameModule::StartupModule()
//{
//	FTextFormatter::Get().RegisterTextArgumentModifier(FTextFormatString::MakeReference(TEXT("index")),
//		[](const FTextFormatString& InArgsString, const FTextFormatPatternDefinitionConstRef& InPatternDef) {
//			return FTextFormatArgumentModifier_IndexForm::Create(InArgsString, InPatternDef);
//		});
//	FTextFormatter::Get().RegisterTextArgumentModifier(FTextFormatString::MakeReference(TEXT("enum")),
//		[](const FTextFormatString& InArgsString, const FTextFormatPatternDefinitionConstRef& InPatternDef) {
//			return FTextFormatArgumentModifier_EnumForm::Create(InArgsString, InPatternDef);
//		});
//}
//
//void FMyGameModule::ShutdownModule()
//{
//	FTextFormatter::Get().UnregisterTextArgumentModifier(FTextFormatString::MakeReference(TEXT("index")));
//	FTextFormatter::Get().UnregisterTextArgumentModifier(FTextFormatString::MakeReference(TEXT("enum")));
//}
//
//		{Value}|index(N1,N2,N3...)
// 
// ローカライズ作業を行う上でも FText 前提で作られているので FString で実装を進めていると痛い目にあいます
// 
// 
// return FText::FromString(FString::FromInt((int)Player->GetHealth()));
// 
//



// UIのTextBoxではBindでGetTextなるものを定義して　　※少なくともGetTextなる部分をc++化しておきたい
// 
// GetKernel->GetLocalizeService->GetText( Ept_AndDest )
// 結果をResultに繋げばUIがローカライズ＆Modifierしてくれる。




