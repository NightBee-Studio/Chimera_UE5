// Fill out your copyright notice in the Description page of Project Settings.


#include "TsMoveComponent.h"

//
// We really really want to change this component.
//
// 
//
//

// Sets default values for this component's properties
UTsMoveComponent::UTsMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UTsMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTsMoveComponent::TickComponent(float dt, ELevelTick type, FActorComponentTickFunction* func)
{
	Super::TickComponent(dt, type, func);

	// ...

//	for(int 1)
	//E_Poison		UMETA(DisplayName = "Poison"),
	//E_Pararise		UMETA(DisplayName = "Pararise"),

}

