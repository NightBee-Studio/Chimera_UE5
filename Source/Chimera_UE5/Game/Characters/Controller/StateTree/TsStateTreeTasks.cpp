// Fill out your copyright notice in the Description page of Project Settings.

#include "TsStateTreeTasks.h"






//-----------------------------------------------------------------------------------------------------------------
//
//
//
//

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








//-----------------------------------------------------------------------------------------------------------------
//
//
//
//


EStateTreeRunStatus 
    FTsTask_Think::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (ctrl) ctrl->Think();
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus 
    FTsTask_Think::Tick(
        FStateTreeExecutionContext&         context,
        const float                         dt
    ) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (ctrl) ctrl->Think();
    return EStateTreeRunStatus::Running;
}





//-----------------------------------------------------------------------------------------------------------------
//
//
//
//

EStateTreeRunStatus 
    FTsTask_DoTagState::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    ATsAIController*    ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return EStateTreeRunStatus::Failed;
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus
    FTsTask_DoTagState::Tick(
        FStateTreeExecutionContext&         context,
        const float                         dt
    ) const
{
    FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
    ATsAIController*    ctrl = Cast<ATsAIController>(context.GetOwner());
    if ( !ctrl ) return EStateTreeRunStatus::Failed;
    if ( !ctrl->HasTag( data.mStatusTag ) ) {
        return EStateTreeRunStatus::Succeeded;
    }
    return EStateTreeRunStatus::Running;
}



EStateTreeRunStatus 
    FTsTask_EnterTagState::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
    ATsAIController*    ctrl = Cast<ATsAIController>(context.GetOwner());
    if ( !ctrl ) return EStateTreeRunStatus::Failed;
    ctrl->AddTag( data.mStatusTag );
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus
    FTsTask_ExitTagState::EnterState(
        FStateTreeExecutionContext&         context,
        const FStateTreeTransitionResult&   trans
    ) const
{
    FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
    ATsAIController*    ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return EStateTreeRunStatus::Failed;
    ctrl->RemoveTag( data.mStatusTag );
    return EStateTreeRunStatus::Succeeded;
}













//-----------------------------------------------------------------------------------------------------------------
//
//
//
//


EStateTreeRunStatus FTsTask_Dummy::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const
{
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FTsTask_Dummy::Tick(FStateTreeExecutionContext& context, const float dt) const
{
    return EStateTreeRunStatus::Running;
}
