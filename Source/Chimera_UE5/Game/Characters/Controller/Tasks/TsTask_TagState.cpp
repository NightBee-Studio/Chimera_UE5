// Fill out your copyright notice in the Description page of Project Settings.

#include "TsTask_TagState.h"
#include "GameFramework/Pawn.h"





EStateTreeRunStatus 
    FTsTask_DoTagState::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    APawn* pawn = Cast<APawn>(context.GetOwner());
    if (pawn){
        FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
        ATsAIController*    ctrl = Cast<ATsAIController>(pawn->GetController());
        if (ctrl) {
            ctrl->AddTag(data.mStatusTag);
            return EStateTreeRunStatus::Running;
        }
    }
    return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus
    FTsTask_DoTagState::Tick(
        FStateTreeExecutionContext& context,
        const float                 dt
    ) const
{
    APawn* pawn = Cast<APawn>(context.GetOwner());
    if ( pawn ) {
        FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
        ATsAIController*    ctrl = Cast<ATsAIController>(pawn->GetController());
        if (ctrl && !ctrl->HasTag(data.mStatusTag)) {
            return EStateTreeRunStatus::Succeeded;
        }
    }
    return EStateTreeRunStatus::Running;
}



EStateTreeRunStatus 
    FTsTask_EnterTagState::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    APawn* pawn = Cast<APawn>(context.GetOwner());
    if (pawn) {
        FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
        ATsAIController*    ctrl = Cast<ATsAIController>(pawn->GetController());
        if (ctrl && ctrl->HasTag(data.mStatusTag)){
            return EStateTreeRunStatus::Running;
        }
    }
    return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus
    FTsTask_ExitTagState::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    APawn* pawn = Cast<APawn>(context.GetOwner());
    if (pawn) {
        FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
        ATsAIController*    ctrl = Cast<ATsAIController>(pawn->GetController());
        if (ctrl) {
            ctrl->RemoveTag(data.mStatusTag);
        }
    }
    return EStateTreeRunStatus::Running;
}
