// Fill out your copyright notice in the Description page of Project Settings.


#include "TsGameSubsystem.h"

#include "GameFramework/Actor.h"

//#include "Item/TsItemSubsystem.h"
//#include "../../System/Config/TsWorldSetting.h"

//
//bool	UTsService::ShouldCreateSubsystem(UObject* owner) const
//{
//#if 1
//	UE_LOG(LogTemp, Log, TEXT("ShouldCreateSubsystem "));
//
//	if (UWorld* w = Cast<UWorld>(owner)) {
//		UE_LOG(LogTemp, Log, TEXT("ShouldCreateSubsystem GetWorld"));
//		if (ATsWorldSetting* ws = Cast<ATsWorldSetting>(w->GetWorldSettings())) {
//			//UE_LOG(LogTemp, Log, TEXT("ShouldCreateSubsystem Index=%d "), ws->mServices.IndexOfByKey(this));
//
//			return ws->mServiceMode == mServiceMode ;
//		}
//	}
//	return false;
//#else
//	return true;
//#endif
//}





static 
class ATsTick
	: public AActor
{
private:
	void	(*mFunc)(float dt)  ;

public:
	void Setup( ETickingGroup tick_grp, void (* fn)(float dt)) {
		PrimaryActorTick.TickGroup = tick_grp;
		PrimaryActorTick.bCanEverTick = true;
		mFunc = fn;
	}

	virtual void Tick(float dt) override{
		mFunc(dt);
	}
};


class TsGameSubsystemImpl {
public:
	TsGameSubsystemImpl(TObjectPtr<UTsGameSubsystem> owner){
#if 0
		mTick_PrePhys  = owner->CreateDefaultSubobject< ATsTick >(TEXT("GameSubsystem_PrePhysics"));
		mTick_PrePhys ->Setup(ETickingGroup::TG_PrePhysics , &SystemTick);
		mTick_PostPhys = owner->CreateDefaultSubobject< ATsTick >(TEXT("GameSubsystem_PostPhysics"));
		mTick_PostPhys->Setup(ETickingGroup::TG_PostPhysics, &SystemTick);
#endif

	}
	TObjectPtr< ATsTick > mTick_PrePhys  ;
	TObjectPtr< ATsTick > mTick_PostPhys ;

	static void SystemTick(float dt) {
	}
};


void UTsGameSubsystem::Initialize( FSubsystemCollectionBase& col )
{
	mImplWork = new TsGameSubsystemImpl( this ) ;
	UE_LOG(LogTemp, Log, TEXT("UTsGameSubsystem Initialize") );

	for (UBlueprint* bp : mServiceBPs) {
		if (bp->GeneratedClass) {
			if (UTsService* sv = bp->GeneratedClass->GetDefaultObject<UTsService>()) {
				mServices.Add(sv);
			}
		}
	}
	for (UTsService* sv : mServices) {
		sv->Initialize(this, col);
	}

}

void UTsGameSubsystem::Deinitialize()
{
	for (UTsService* sv : mServices) {
		sv->Deinitialize();
	}

	if (mImplWork != nullptr) {
		delete mImplWork;
		mImplWork = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("UTsGameSubsystem Deinitialize"));
}


bool UTsGameSubsystem::IsTickable() const
{
	for (UTsService* sv : mServices) {
		//sv->Deinitialize();
	}

	return true;
}

TStatId UTsGameSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTsGameSubsystem, STATGROUP_Tickables);
}

void UTsGameSubsystem::Tick(float dt)
{
	//UE_LOG(LogTemp, Log, TEXT("UTsGameSubsystem ExecuteTick"));
}



#if 0
USTRUCT()
struct FAFMessageTick : public FTickFunction
{
	GENERATED_BODY()

	/**  AActor  that is the target of this tick **/
	class UGAAbilitiesComponent* Target;

	/**
	* Abstract function actually execute the tick.
	* @param DeltaTime - frame time to advance, in seconds
	* @param TickType - kind of tick for this frame
	* @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
	* @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completetion of this task until certain child tasks are complete.
	**/
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
};

void FAFMessageTick::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target && !Target->IsPendingKillOrUnreachable() && Target->bRegistered)
	{
		if (TickType != LEVELTICK_ViewportsOnly || TickType == LEVELTICK_ViewportsOnly)
		{
			FScopeCycleCounterUObject ActorScope(Target);
			Target->TickMessageQueue(DeltaTime, TickType, *this);
		}
	}
}

FString FAFMessageTick::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[TickAction]");
}


UPROPERTY()
FAFMessageTick MessageQueueTick;


UGAAbilitiesComponent::UGAAbilitiesComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	bIsAnyAbilityActive = false;
	bAutoActivate = true;
	//bAutoRegister = true;
	/*PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bRunOnAnyThread = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;*/


	MessageQueueTick.bCanEverTick = true;
	MessageQueueTick.bRunOnAnyThread = false;
	MessageQueueTick.bAllowTickOnDedicatedServer = true;
	MessageQueueTick.TickGroup = ETickingGroup::TG_PrePhysics;
}

void UGAAbilitiesComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	SetupActorComponentTickFunction(&MessageQueueTick);
	MessageQueueTick.Target = this;
	MessageQueueTick.RegisterTickFunction(GetWorld()->PersistentLevel);
	MessageQueueTick.SetTickFunctionEnable(bRegister);
}

#endif