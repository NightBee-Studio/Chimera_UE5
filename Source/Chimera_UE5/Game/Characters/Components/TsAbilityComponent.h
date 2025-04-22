// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "TsAbilityComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHIMERA_UE5_API UTsAbility : public UGameplayAbility
{
	GENERATED_BODY()

	// AbilitySystemComponent��GameplayTagCountContainer�ɐV����GameplayTag��ǉ�����
	UFUNCTION(BlueprintCallable, Category = "GamePlayAbility")
	virtual void AddGameplayTags(const FGameplayTagContainer GameplayTags);

	// AbilitySystemComponent��GameplayTagCountContainer��GameplayTag���폜����
	UFUNCTION(BlueprintCallable, Category = "GamePlayAbility")
	virtual void RemoveGameplayTags(const FGameplayTagContainer GameplayTags);
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHIMERA_UE5_API UTsAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UTsAbilityComponent();

	//UPROPERTY(BlueprintReadWrite, Category = "AbilitySystem")
//	TList<TObjectPtr<UTsAbility>>		mAbilityList;
	//TList<TSubclassOf<UTsAbility>>		mAbilityList;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float dt, ELevelTick type, FActorComponentTickFunction* func) override;

};


