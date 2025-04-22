// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "TsStateTreeComponent.generated.h"


UCLASS( ClassGroup=(AI), meta=(BlueprintSpawnableComponent) )
class CHIMERA_UE5_API UTsStateTreeComponent
	: public UStateTreeComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTsStateTreeComponent(const FObjectInitializer& obj_init );

	void SetStateTree(UStateTree* state_tree );

};
