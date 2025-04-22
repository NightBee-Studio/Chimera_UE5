// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../TsCharacter.h"
#include "TsCharaPlayer.generated.h"



UCLASS(Transient, Blueprintable, ClassGroup = (Custom), meta = (BlueprintThreadSafe))
class CHIMERA_UE5_API UTsPlayerParam
	: public UTsCharaParam
{
	GENERATED_BODY()

};


UCLASS( ClassGroup=(Custom) )
class CHIMERA_UE5_API ATsCharaPlayer : public ATsCharacter
{
	GENERATED_BODY()

public:
	ATsCharaPlayer();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick( float dt ) override;

	virtual UTsCharaParam* CreateCharaParam( const FName& name ) override;
};
