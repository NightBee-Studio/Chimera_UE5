// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "StateTreeTaskBase.h"
#include "StateTreeReference.h"
#include "StateTreeExecutionContext.h"
#include "Components/StateTreeComponent.h"

#include "TsTask_Test.generated.h"



USTRUCT(BlueprintType, meta = (DisplayName = "Do Test(Ts)"))
struct CHIMERA_UE5_API FTsTask_Dummy
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    virtual EStateTreeRunStatus EnterState  (FStateTreeExecutionContext& context, const FStateTreeTransitionResult& trans   ) const override;
    virtual EStateTreeRunStatus Tick        (FStateTreeExecutionContext& context, const float dt                            ) const override;
};

