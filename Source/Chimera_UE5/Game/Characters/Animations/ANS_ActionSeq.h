// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_ActionSeq.generated.h"


//Received_NotifyBegin	通知開始イベント
//Received_NotifyEnd	通知終了イベント
//Received_NotifyTick	開始と終了の間、アニメーションがアップデートされる度に実行されるイベント
//GetNotifyName			Animation Sequence内で表示されるNotify名 上書きしない場合はクラス名


/**
 * 
 */
UCLASS()
class CHIMERA_UE5_API UANS_ActionSeq : public UAnimNotifyState
{
	GENERATED_BODY()
	
};


