// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsService.generated.h"


template <std::size_t N>
constexpr int32_t Hash(const char(&str)[N], std::size_t i = 0, int32_t hash = 5381) {
	return str[i] == '\0' ? hash : Hash(str, i + 1, hash * 33 + str[i]);
}


using TsHandler = int;



UENUM(BlueprintType, Meta = (Bitflags))
enum EServiceFlag : uint8 {
	E_TickPrePhys,
	E_TickPostPhys,
};
ENUM_CLASS_FLAGS(EServiceFlag);


UENUM(BlueprintType)
enum EServiceMode : uint8 {
	E_AnyMode,

	E_TitleMode,
	E_GameMode,
	E_HomeMode,
	E_GuildMode,
};


UCLASS(Blueprintable, Abstract)
class CHIMERA_UE5_API UTsService
	: public UObject
{
	GENERATED_BODY()

private:
	TObjectPtr<USubsystem> mOwner;


public:
	UTsService() {}

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Service", Meta = (DisplayName = "Service Mode"))
	TEnumAsByte<EServiceMode>	mServiceMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Service", Meta = (DisplayName = "Flag", Bitmask, BitmaskEnum = "EServiceFlag"))
	int32						mFlag;

	template<typename T> T*	GetOwner() { return  Cast<T>(mOwner); }

	virtual void	Initialize( USubsystem* owner, FSubsystemCollectionBase& collection) { mOwner = owner; }
	virtual void	Deinitialize() {}

	virtual void	Update( EServiceFlag f ) {}
};

