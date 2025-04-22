// Fill out your copyright notice in the Description page of Project Settings.

#include "TsAbilityComponent.h"


void UTsAbility::AddGameplayTags(const FGameplayTagContainer GameplayTags)
{
	UAbilitySystemComponent* comp = GetAbilitySystemComponentFromActorInfo();

	comp->AddLooseGameplayTags(GameplayTags);
}

void UTsAbility::RemoveGameplayTags(const FGameplayTagContainer GameplayTags)
{
	UAbilitySystemComponent* comp = GetAbilitySystemComponentFromActorInfo();

	comp->RemoveLooseGameplayTags(GameplayTags);
}



UTsAbilityComponent::UTsAbilityComponent()
{
}

void UTsAbilityComponent::BeginPlay() 
{
}
	// Called every frame
void UTsAbilityComponent::TickComponent(float dt, ELevelTick type, FActorComponentTickFunction* func)
{
}

