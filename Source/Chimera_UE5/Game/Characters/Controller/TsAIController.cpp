// Fill out your copyright notice in the Description page of Project Settings.


#include "TsAIController.h"
#include "../Components/TsStateTreeComponent.h"

#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "NativeGameplayTags.h"



namespace ATsAIControllerTag
{
    // Tag for main AI-status 
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus,            "AIStatus"            );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_Normal     , "AIStatus.Normal"     );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_NormalSkip , "AIStatus.NormalSkip" );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_Caution    , "AIStatus.Caution"    );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_CautionSkip, "AIStatus.CautionSkip");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_Attack     , "AIStatus.Attack"     );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_AttackSkip , "AIStatus.AttackSkip" );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_Dead       , "AIStatus.Dead"       );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_DeadSkip   , "AIStatus.DeadSkip"   );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_Halt       , "AIStatus.Halt"       );
    UE_DEFINE_GAMEPLAY_TAG_STATIC(AIStatus_Init       , "AIStatus.Init"       );
}




void ATsAIController::Think()
{
    //if ( mStatusTags.HasTagExact(ATsAIControllerTag::AIStatus_Normal) )
    if ( mAI_Vigilance < 0.2f ){
        AddTag( ATsAIControllerTag::AIStatus_Normal );
    }
}




ATsAIController::ATsAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATsAIController::BeginPlay()
{
    Super::BeginPlay();

    mAI_Vigilance = 0;

    //// �^�[�Q�b�g�����x�����̃A�N�^�[����擾�i��FTag��"Target"��Actor�j
    //TArray<AActor*> FoundActors;
    //UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Target"), FoundActors);

    //if (FoundActors.Num() > 0)
    //{
    //    TargetActor = FoundActors[0];
    //    MoveToActor(TargetActor, 100.0f); // 100���j�b�g�ȓ��ɋ߂Â��Γ��B�Ƃ݂Ȃ�
    //}

}

void ATsAIController::OnPossess( APawn* pawn )
{
    Super::OnPossess(pawn);

    UStateTreeComponent* st_comp = FindComponentByClass<UStateTreeComponent>();
    st_comp->StartLogic();
}

void ATsAIController::Tick(float dt)
{
    Super::Tick(dt);

    //// ���B���Ă��Ȃ���Βǂ�������
    //if (mTargetActor && !LineOfSightTo(mTargetActor))
    //{
    //    MoveToActor(mTargetActor);
    //}
}


