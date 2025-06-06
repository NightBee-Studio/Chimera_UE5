// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "StateTreeReference.h"
#include "StateTreeExecutionContext.h"

#include "../../Components/TsStatusComponent.h"
#include "../TsAIController.h"
#include "TsStateTreeData.h"

#include "TsStateTreeConditions.generated.h"






USTRUCT(BlueprintType, meta = (DisplayName = "Check CharaState(Ts)"))
struct CHIMERA_UE5_API FTsCond_CharaState
    : public FStateTreeConditionCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_StateChara;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_StateChara::StaticStruct();
    }
    virtual bool TestCondition(FStateTreeExecutionContext& context) const override;
};


USTRUCT(BlueprintType, meta = (DisplayName = "Check StatusTag(Ts)"))
struct CHIMERA_UE5_API FTsCond_StatusTag
    : public FStateTreeConditionCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_DoTagState;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_DoTagState::StaticStruct();
    }
    virtual bool TestCondition(FStateTreeExecutionContext& context) const override;
};
