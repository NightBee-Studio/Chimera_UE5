// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "GameFramework/Pawn.h"
//#include "Abilities/GameplayAbility.h"

#include "GameFramework/Character.h"
#include "Components/SceneComponent.h" 
#include "Components/SkeletalMeshComponent.h" 
#include "Components/CapsuleComponent.h" 
#include "Components/StateTreeComponent.h"

#include "System/TsSystem.h"
#include "Components/TsStatusComponent.h"
#include "Components/TsMoveComponent.h"
#include "Components/TsCombatComponent.h"
#include "Components/TsSkillComponent.h"
#include "Components/TsAbilityComponent.h"

#include "CharaAction.h"

#include "TsCharacter.generated.h"




#define TsCharaType(nm)	ETch_##nm	UMETA(DisplayName = #nm)

UENUM(BlueprintType)
enum class ECharaType : uint8 {
	//TsCharaType(Player),
	ETch_Player	UMETA(DisplayName = "Player"),
	ETch_PlMale			UMETA(DisplayName = "PlMale"),
	ETch_PlFemale		UMETA(DisplayName = "PlFemale"),

	ETch_Npc	UMETA(DisplayName = "NPC"),
	ETch_NpGeorge		UMETA(DisplayName = "NpGeorge"),

	ETch_Mob	UMETA(DisplayName = "EnemyMob"),
	ETch_MbGoblin		UMETA(DisplayName = "MbGoblin"),
	ETch_MbHobgoblin	UMETA(DisplayName = "MbHobgoblin"),
	ETch_MbKobolt		UMETA(DisplayName = "MbKobolt"),
	ETch_MbGiant		UMETA(DisplayName = "MbGiant"),

	ETch_Elite	UMETA(DisplayName = "EnemyElite"),
	ETch_ElMinotaur		UMETA(DisplayName = "ElMinotaur"),
	ETch_ElCocatoris	UMETA(DisplayName = "ElCocatoris"),

	ETch_Boss	UMETA(DisplayName = "EnemyBoss"),
	ETch_BsDragon		UMETA(DisplayName = "BsDragon"),
};

#undef TsCharaType




USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FCharaSpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Type"))
	ECharaType				mType;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Bitmask, BitmaskEnum = "EItemFlag", DisplayName = "Flag"))
	//int32					mFlag;

	//AI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "AIStateTree"))
	TObjectPtr<UStateTree>	mAIStateTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "CharaObj"))
	TObjectPtr<ATsCharacter>	mChara;
};


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintThreadSafe))
class CHIMERA_UE5_API UTsCharaParam
	: public UTsParams
{
	GENERATED_BODY()

private:
	// backbuffer
	ECharaAction	mActionReq;
	FVector			mPosReq;
	float			mRotReq;
	float			mSpeedReq;

public:
	UTsCharaParam(const FObjectInitializer& init);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "CharaParam")
	ECharaAction	mAction;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "CharaParam")
	FVector			mPos;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "CharaParam")
	float			mRot;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "CharaParam")
	float			mSpeed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void			SetAction(ECharaAction id)	{ mActionReq = id   ; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void			SetSpeed(float speed)		{ mSpeedReq  = speed; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void			SetPos(const FVector& pos)	{ mPosReq    = pos  ; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void			SetRot(float rot)			{ mRotReq    = rot  ; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void			Update();

	// Animation Condition
public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool IsActionRange(ECharaAction a, ECharaAction b) const { return mAction >= a && mAction <= b; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool IsAction(ECharaAction a) const					{ return mAction == a ; }
};








UCLASS()
class CHIMERA_UE5_API ATsCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATsCharacter(const FObjectInitializer& obj_init = FObjectInitializer::Get());

	// UE Service Functions
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float dt) override;
	virtual void PossessedBy(AController* controller) override;

	// Components
public:
	TObjectPtr<USceneComponent       >		mSceneRoot   ;
	TObjectPtr<USkeletalMeshComponent>		mSkeletalMesh;
	TObjectPtr<UCapsuleComponent     >		mCapsule ;

	TObjectPtr<UTsMoveComponent   >			mMoveCmp   ;
	TObjectPtr<UTsStatusComponent >			mStatusCmp ;
	TObjectPtr<UTsCombatComponent >			mCombatCmp ;
	TObjectPtr<UTsSkillComponent  >			mSkillCmp;
	TObjectPtr<UTsAbilityComponent>			mAbilityCmp;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	UAbilitySystemComponent*	GetAbilityComponent() const	{ return mAbilityCmp ; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	UTsSkillComponent*			GetSkillComponent()	const	{ return mSkillCmp   ; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	UTsCombatComponent*			GetCombatComponent() const	{ return mCombatCmp  ; }
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	UTsStatusComponent*			GetStatusComponent() const	{ return mStatusCmp  ; }

	// Parameters
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	TArray<TSubclassOf<class UTsAbility>>	mAbilityList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharaParam")
	TObjectPtr <UTsCharaParam>				mParam;
	virtual UTsCharaParam*		CreateCharaParam(const FName& name);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE UTsCharaParam*	GetCharaParam() const { return mParam; }

	// Accessor
public:
	UFUNCTION(BlueprintCallable)
	virtual void				SetAction(ECharaAction id) ;
	UFUNCTION(BlueprintCallable)
	virtual ECharaAction		GetAction();
	UFUNCTION(BlueprintCallable)
	virtual void				SetDir(FVector2D dir);
};

