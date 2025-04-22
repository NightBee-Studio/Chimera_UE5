// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "StateTreeTaskBase.h"
#include "StateTreeReference.h"
#include "StateTreeExecutionContext.h"

#include "../../Components/TsStatusComponent.h"
#include "../TsAIController.h"

#include "TsTask_CharaState.generated.h"





USTRUCT(BlueprintType, meta = (DisplayName = "Wait CharaState(Ts)"))
struct CHIMERA_UE5_API FTsTask_WaitForCharaState
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_StateChara;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_StateChara::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const override {
        return EStateTreeRunStatus::Running;
    }
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& context, const float dt) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Set CharaState(Ts)"))
struct CHIMERA_UE5_API FTsTask_SetCharaState
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_StateChara;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_StateChara::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const override{
        return EStateTreeRunStatus::Running;
    }
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& context, const float dt) const override;
};
