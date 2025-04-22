// Fill out your copyright notice in the Description page of Project Settings.


#include "TsStateTreeComponent.h"


UTsStateTreeComponent::UTsStateTreeComponent( const FObjectInitializer& obj_init )
	: Super(obj_init)
{
	
}

void UTsStateTreeComponent::SetStateTree( UStateTree* state_tree )
{
	if (state_tree){
		StateTreeRef.SetStateTree(state_tree);
	}
}

