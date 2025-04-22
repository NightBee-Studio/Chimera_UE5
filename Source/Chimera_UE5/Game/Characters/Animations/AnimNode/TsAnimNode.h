// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../CharaAction.h"
//#include "AnimGraph/AnimGraphNode_Base.h"
#include "Animation/AnimNodeBase.h"
#include "TsAnimNode.generated.h"


//UAnimStateNode

/**
 * 
 */

USTRUCT(BlueprintType)
//struct FTsAnimGraphNode : public UAnimGraphNode_Base
struct FTsAnimGraphNode : public FAnimNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "AnimGraph")
	ECharaAction		mActionID;

	//UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	//void					SetCharaParam(UTsCharaParam* param) { mCharaParam = param; }
};
