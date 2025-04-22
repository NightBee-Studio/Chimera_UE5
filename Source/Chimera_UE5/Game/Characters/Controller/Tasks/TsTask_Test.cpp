// Fill out your copyright notice in the Description page of Project Settings.

#include "TsTask_Test.h"


EStateTreeRunStatus FTsTask_Dummy::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans) const
{
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FTsTask_Dummy::Tick(FStateTreeExecutionContext& context, const float dt) const
{
    return EStateTreeRunStatus::Running;
}
