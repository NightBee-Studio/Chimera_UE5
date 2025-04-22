// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNodeBase.h"

#include "../CharaAction.h"
#include "../TsCharacter.h"
#include "TsAnimInstance.generated.h"


//
// Loop State must have the following UpdateFunctions.
//	- Select the SequnecePlayer and select Functions->OnInitialUpdate
//  - Bind the new function.
//  - Linked to the node of 'OnInitialUpdate_LoopState()', and the set the Loop State for the argument
//


//
//UCLASS()
//class CHIMERA_UE5_API UAnimGraphNode_MySequencePlayer
//	: public UAnimGraphNode_Base
//{
//	GENERATED_BODY()
//
//	UPROPERTY(EditAnywhere, Category = Settings)
//	FAnimNode_SequencePlayer Node;
//};


UCLASS()
class CHIMERA_UE5_API UTsAnimInstance
	: public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "CharaParam")
	UTsCharaParam* mCharaParam;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void						SetCharaParam(UTsCharaParam* param) {
		mCharaParam = param;
	}

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE UTsCharaParam *	GetCharaParam() const {
		ATsCharacter* chara = Cast<ATsCharacter>(TryGetPawnOwner());
		return (chara ? chara->GetCharaParam() : nullptr);
	}

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE UTsStatusComponent* GetCharaStatus() const {
		ATsCharacter* chara = Cast<ATsCharacter>(TryGetPawnOwner());
		return (chara ? chara->GetStatusComponent() : nullptr);
	}


	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnInitialUpdate_LoopState( ECharaAction loop_act ) { 
		ATsCharacter* chara = Cast<ATsCharacter>(TryGetPawnOwner());
		if ( chara ) chara->GetCharaParam()->SetAction(loop_act);
	}

};
