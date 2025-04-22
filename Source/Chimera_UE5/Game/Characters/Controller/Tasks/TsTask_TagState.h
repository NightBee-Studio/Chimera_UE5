// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "StateTreeTaskBase.h"
#include "StateTreeReference.h"
#include "StateTreeExecutionContext.h"

#include "../TsAIController.h"

#include "TsTask_TagState.generated.h"






USTRUCT(BlueprintType, meta = (DisplayName = "Do TagState(Ts)"))
struct CHIMERA_UE5_API FTsTask_DoTagState
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_DoTagState;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_DoTagState::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& context, const float dt) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Enter TagState(Ts)"))
struct CHIMERA_UE5_API FTsTask_EnterTagState
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_DoTagState;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_DoTagState::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& context, const float dt) const override {
        return EStateTreeRunStatus::Succeeded;
    }
};

USTRUCT(BlueprintType, meta = (DisplayName = "Exit TagState(Ts)"))
struct CHIMERA_UE5_API FTsTask_ExitTagState
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FTsData_DoTagState;
    virtual const UStruct* GetInstanceDataType() const override {
        return FTsData_DoTagState::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& context, const float dt) const override {
        return EStateTreeRunStatus::Succeeded;
    }

};









