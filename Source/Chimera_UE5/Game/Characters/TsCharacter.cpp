// Fill out your copyright notice in the Description page of Project Settings.


#include "TsCharacter.h"
#include "Animations/TsAnimInstance.h"


UTsCharaParam::UTsCharaParam(const FObjectInitializer& init)
	: Super(init)
{
//	mReqBuffer = CreateDefaultSubobject<UTsCharaParamCore>(TEXT("ReqBuffer"));
	//mBackbuffer = NewObject<UTsCharaParam>(this, classof->GetFName(), RF_NoFlags, classof->GetDefaultObject());;		//write
}

// copy the buffer
void	UTsCharaParam::Update()
{
	float d;
	if (mAction != mActionReq) {
		mAction = mActionReq;
	}

	d = (mRotReq - mRot) * 0.3f;
	if (d > 0.00001f || d < -0.00001f) {
		mRot += d ;
	}

	d = (mSpeedReq - mSpeedReq) * 0.1f;
	if (d > 0.00001f || d < -0.00001f) {
		mSpeed += d;
	}
}



// Sets default values
ATsCharacter::ATsCharacter(const FObjectInitializer& obj_init)
	: Super(obj_init.SetDefaultSubobjectClass<UTsMoveComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

//	mParam		 = CreateParam();
//	mParamReq	 = CreateParam();

	//mParam = NewObject<UTsCharaParam>(this, classof->GetFName(), RF_NoFlags, classof->GetDefaultObject());
	mParam = CreateCharaParam(TEXT("CharaParam"));

	//mSceneRoot	= CreateDefaultSubobject<USceneComponent       >(TEXT("SceneComponent"       ));
	//mCapsule      = CreateDefaultSubobject<UCapsuleComponent     >(TEXT("CapsuleComponent"     ));
	//mCapsule     ->SetupAttachment(mSceneRoot);
	//mSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	//mSkeletalMesh->SetupAttachment(mCapsule);
	//mCharaMove    = CreateDefaultSubobject<UTsMoveComponent   >(TEXT("MoveComponent"    ));
	//mCharaMove   ->SetUpdatedComponent( mCapsule );
	mStatusCmp  = CreateDefaultSubobject<UTsStatusComponent >(TEXT("StatusComponent" ));
	mCombatCmp  = CreateDefaultSubobject<UTsCombatComponent >(TEXT("CombatComponent" ));
	mSkillCmp   = CreateDefaultSubobject<UTsSkillComponent  >(TEXT("SkillComponent"  ));
	mAbilityCmp = CreateDefaultSubobject<UTsAbilityComponent>(TEXT("AbilityComponent"));
}

void ATsCharacter::PossessedBy(AController* controller)
{
	Super::PossessedBy(controller);

	mAbilityCmp->RefreshAbilityActorInfo();
}

// Called when the game starts or when spawned
void ATsCharacter::BeginPlay()
{
	Super::BeginPlay();

	// initialize component
	if ( UTsAnimInstance* ai = Cast<UTsAnimInstance>(GetMesh()->GetAnimInstance() ) ){
		ai->SetCharaParam( mParam );
	}

	if (mAbilityCmp){// initialize
		int id = 0;
		for (auto ab : mAbilityList) {
			if (ab) mAbilityCmp->GiveAbility( FGameplayAbilitySpec(ab.GetDefaultObject(), 1, id++ ) );
		}
		mAbilityCmp->InitAbilityActorInfo(this, this);
	}
}

// Called every frame
void ATsCharacter::Tick(float dt)
{
	Super::Tick(dt);

	mParam->Update();

	SetActorRotation(FRotator(0, mParam->mRot, 0));
	if (mParam->mSpeed > 0.00001f || mParam->mSpeed < -0.00001f) {
		AddMovementInput(GetActorTransform().GetRotation().GetForwardVector(), mParam->mSpeed, false);
	}
}

UTsCharaParam* ATsCharacter::CreateCharaParam(const FName& name)
{
	return CreateDefaultSubobject<UTsCharaParam>( name );
}

//
//FCharaParam* ATsCharacter::CreateParam()
//{
//	return new FCharaParam;
//}

void ATsCharacter::SetAction(ECharaAction id)
{
	mParam->mAction = id;
}

ECharaAction  ATsCharacter::GetAction()
{
	return mParam->mAction ;
}

void ATsCharacter::SetDir(FVector2D dir)//
{
	float rot = atan2f(dir.Y, dir.X) / PI * 180.0f;

	float d = (rot - mParam->mRot) * 0.3f;
	if (d > 0.00001f || d < -0.00001f) {
		rot += d;
	}

	mParam->mRot = rot;

	UE_LOG(LogTemp, Log, TEXT("SetDir = %f (%f %f)"), mParam->mRot, dir.X, dir.Y );
}

