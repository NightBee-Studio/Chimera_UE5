// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AIController.h"
#include "Components/StateTreeComponent.h"

#include "../CharaAction.h"
#include "../TsCharacter.h"

#include "TsAIController.generated.h"



UCLASS()
class CHIMERA_UE5_API ATsAIController
	: public AAIController
{
	GENERATED_BODY()

public:
	ATsAIController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* pawn) override;

public:
	virtual void Tick(float dt) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	ATsCharacter*						mTargetChara;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float								mAI_Vigilance ;//

	//

	// Tage
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FGameplayTagContainer				mStatusTags;


	void Think();


	void AddTag		(FGameplayTag tag)		{ mStatusTags.AddTag(tag); }
	void RemoveTag	(FGameplayTag tag)		{ mStatusTags.RemoveTag(tag); }
	bool HasTag		(FGameplayTag tag) const{ return mStatusTags.HasTag(tag); }
};


