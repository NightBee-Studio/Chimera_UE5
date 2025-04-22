// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"



AGameCamera::AGameCamera()
	//: Super(CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent")))
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mLength = 100.0f;
	mView = FVector2D(0, 0);

	//USpringArmComponent *sp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	//sp->SetupAttachment( GetDefaultAttachComponent() );
//	sp->
	//sp->TargetArmLength = 100.0f;
}

void AGameCamera::TickActor(float dt, enum ELevelTick tick_type, FActorTickFunction& func )
{
	Super::TickActor(dt, tick_type, func);

//	UE_LOG( LogTemp, Log, TEXT("Cam Test") );
	if (mTarget){
		//GetCameraComponent()->SetViewTarget(mTarget);

		FRotator   rot = FRotator(0, mView.Y, mView.X);
		FTransform trs = FTransform(rot, mTarget->GetActorLocation() + rot.RotateVector(FVector(0, mLength, 0)));

		SetActorTransform( trs ) ;
		GetCameraComponent()->SetComponentToWorld( trs );
	}
}

void AGameCamera::SetTarget(APawn* chara)
{
	mTarget = chara;
}

void AGameCamera::SetView( const FVector2D &dir )
{
//	mView += (dir * PI - mView) * 0.2f;
	mView = dir * PI / 180.0f;

	UE_LOG( LogTemp, Log, TEXT("CamView  dir(%f %f)=>View(%f %f)"), dir.X,dir.Y, mView.X, mView.Y );
}

void AGameCamera::SetLength(float leng)
{
	mLength = leng;
}
