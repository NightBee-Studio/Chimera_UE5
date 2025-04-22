// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AIController.h"
#include "Components/StateTreeComponent.h"

#include "../CharaAction.h"
#include "../TsCharacter.h"

#include "TsAIController.generated.h"




USTRUCT()
struct CHIMERA_UE5_API FTsData_StateChara
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (DisplayName = "CharaStatus"))
	ECharaStatus mCharaStatus;
};

USTRUCT()
struct CHIMERA_UE5_API FTsData_DoTagState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (DisplayName = "StatusTag"))
	FGameplayTag mStatusTag;

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (DisplayName = "SkipTag"))
	FGameplayTag mSkipTag;
};





USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FAISpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Mesh"))
	TObjectPtr<UStateTree>	mStateTree;
};


UCLASS()
class CHIMERA_UE5_API ATsAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATsAIController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float dt) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable>				mDatas;

	UPROPERTY(EditAnywhere)
	ATsCharacter*						mTargetActor;

	// Tage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer				mStatusTags;

	void AddTag		(FGameplayTag tag)		{ mStatusTags.AddTag(tag); }
	void RemoveTag	(FGameplayTag tag)		{ mStatusTags.RemoveTag(tag); }
	bool HasTag		(FGameplayTag tag) const{ return mStatusTags.HasTag(tag); }
};
