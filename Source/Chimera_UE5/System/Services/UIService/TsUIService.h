// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "System/TsSystem.h"

#include "TsUIService.generated.h"

//
// SaveData for local-save
// Net Transfer for DB.
//


UENUM(BlueprintType)
enum EUIFlag : uint8 {
	EFui_None,
	EFui_Single,
	EFui_Modal,
};

UENUM(BlueprintType)
enum EUISlot : uint8 {
	ESui_Default,
	ESui_Dialog,
};

UENUM(BlueprintType)
enum EUIID {
	EIDui_None,
	EIDui_Loading,
	EIDui_Fade,
};



USTRUCT()
struct CHIMERA_UE5_API FTsUISpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Flag"    , Bitmask, BitmaskEnum = "EUIFlag"))
	int32				mFlag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Meta = (DisplayName = "Priority"))
	int32				mPriority;
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Meta = (DisplayName = "Slot"    ))
	int32				mSlot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Meta = (DisplayName = "UI"      ))
	TObjectPtr<UObject>	mUI;
};


using TsHandler = int;

UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsWidget
	: public UUserWidget
{
	GENERATED_BODY()
private:
	static int	gID;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (DisplayName = "ID"))
	int			mID;

	UTsWidget(const FObjectInitializer& obj_init);
	~UTsWidget();

	UFUNCTION(BlueprintCallable)
	FText			GetLocalizedText(const FName &key) const;

	virtual bool	Initialize() override;

	void	Play() {}
	void	Pause(bool stop) {}
	void	Stop() {}
};



UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsUIService
	: public UTsService
{
	GENERATED_BODY()

public:
	UTsUIService()
		: UTsService()	{}
	~UTsUIService()		{}

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UIService", Meta = (DisplayName = "Specs", RowType = "FTsUISpec"))
	TObjectPtr<UDataTable>				mSpecs;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UIService", Meta = (DisplayName = "Widgets", RowType = "FTsUISpec"))
	TArray<TObjectPtr<UTsWidget>>		mWidgets;

	virtual void	Initialize( USubsystem* owner, FSubsystemCollectionBase& collection) override;
	virtual void	Deinitialize() override;
	virtual void	Update( EServiceFlag f ) override {}

public:
	void			Add   (UTsWidget* w)	{ if (w) mWidgets.Add   (w); }
	void			Remove(UTsWidget* w)	{ if (w) mWidgets.Remove(w); }

public:
	TsHandler		Open (EUIID ui) ;
	void			Close(EUIID ui) ;

	UTsWidget*		Get(TsHandler id) ;

	void			Play (EUIID ui) {}
	void			Pause(EUIID ui, bool stop) {}
	void			Stop (EUIID ui) {}
};

