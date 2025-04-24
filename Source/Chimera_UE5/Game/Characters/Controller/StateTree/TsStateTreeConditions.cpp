// Fill out your copyright notice in the Description page of Project Settings.

#include "TsStateTreeConditions.h"

bool FTsCond_CharaState::TestCondition(FStateTreeExecutionContext& context) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return false;

    ATsCharacter* chara = Cast<ATsCharacter>(ctrl->GetPawn());
    if (!chara) return false;
    
    FTsData_StateChara& data = context.GetInstanceData<FTsData_StateChara>(*this);
    return chara->GetStatusComponent()->Is(data.mCharaStatus);
}


bool FTsCond_StatusTag::TestCondition(FStateTreeExecutionContext& context) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return false;

    FTsData_DoTagState& data = context.GetInstanceData<FTsData_DoTagState>(*this);
    return ctrl->HasTag( data.mStatusTag );
}

