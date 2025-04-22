// Fill out your copyright notice in the Description page of Project Settings.

#include "TsTask_CharaState.h"

EStateTreeRunStatus
    FTsTask_WaitForCharaState::Tick(
        FStateTreeExecutionContext& context,
        const float                 dt
    ) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return EStateTreeRunStatus::Failed;

    ATsCharacter* chara = Cast<ATsCharacter>(ctrl->GetPawn());
    if (!chara) return EStateTreeRunStatus::Failed;

    FTsData_StateChara& data = context.GetInstanceData<FTsData_StateChara>(*this);
    if ( chara->GetStatusComponent()->Is( data.mCharaStatus ) ) {
        return EStateTreeRunStatus::Succeeded;
    }
    return EStateTreeRunStatus::Running;
}


EStateTreeRunStatus
    FTsTask_SetCharaState::Tick(
        FStateTreeExecutionContext& context,
        const float                 dt
    ) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return EStateTreeRunStatus::Failed;

    ATsCharacter* chara = Cast<ATsCharacter>(ctrl->GetPawn());
    if (!chara) return EStateTreeRunStatus::Failed;

    FTsData_StateChara& data = context.GetInstanceData<FTsData_StateChara>(*this);
    chara->GetStatusComponent()->Set(data.mCharaStatus) ;

    return EStateTreeRunStatus::Succeeded;
}

