// Fill out your copyright notice in the Description page of Project Settings.


#include "TsCombatComponent.h"


// Sets default values for this component's properties
UTsCombatComponent::UTsCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTsCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTsCombatComponent::TickComponent(float df, ELevelTick type, FActorComponentTickFunction* func)
{
	Super::TickComponent(df, type, func);

}
