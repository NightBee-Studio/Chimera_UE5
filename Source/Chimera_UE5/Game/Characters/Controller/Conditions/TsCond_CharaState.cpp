// Fill out your copyright notice in the Description page of Project Settings.

#include "TsCond_CharaState.h"

bool FTsCond_CharaState::TestCondition(FStateTreeExecutionContext& context) const
{
    ATsAIController* ctrl = Cast<ATsAIController>(context.GetOwner());
    if (!ctrl) return false;

    ATsCharacter* chara = Cast<ATsCharacter>(ctrl->GetPawn());
    if (!chara) return false;
    
    FTsData_StateChara& data = context.GetInstanceData<FTsData_StateChara>(*this);
    return chara->GetStatusComponent()->Is(data.mCharaStatus);
}

