// Fill out your copyright notice in the Description page of Project Settings.


#include "TsCharaPlayer.h"


ATsCharaPlayer::ATsCharaPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ATsCharaPlayer::BeginPlay()
{
	Super::BeginPlay();
}


void ATsCharaPlayer::Tick(float dt)
{
	Super::Tick(dt);
}

UTsCharaParam* ATsCharaPlayer::CreateCharaParam( const FName &name )
{
	return Cast<UTsCharaParam>(CreateDefaultSubobject<UTsPlayerParam>( name ));
}

