// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharaAction.generated.h"

UENUM(BlueprintType)
enum class ECharaAction : uint8 {
	EAch_Idle	UMETA(DisplayName = "Idle"   ),
	EAch_Move	UMETA(DisplayName = "Move"   ),
	EAch_Jump	UMETA(DisplayName = "Jump"   ),
	EAch_Fall	UMETA(DisplayName = "Fall"   ),
	EAch_Attack	UMETA(DisplayName = "Attack"),
		EAch_AtkMeleeA	UMETA(DisplayName = "AtkMeleeA"),
		EAch_AtkMeleeB	UMETA(DisplayName = "AtkMeleeB"),
		EAch_AtkMeleeC	UMETA(DisplayName = "AtkMeleeC"),
		EAch_AtkSmash	UMETA(DisplayName = "AtkSmash" ),
	EAch_Damage	UMETA(DisplayName = "Damage"),
		EAch_DmgFront	UMETA(DisplayName = "DmgFront" ),
		EAch_DmgBack	UMETA(DisplayName = "DmgBack"  ),

	EAch_Stun	UMETA(DisplayName = "Stun"),
};
//ENUM_CLASS_FLAGS(ECharaAction);

