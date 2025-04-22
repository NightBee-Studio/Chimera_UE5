// Fill out your copyright notice in the Description page of Project Settings.


#include "TsStatusComponent.h"


UTsStatusComponent::UTsStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	mStatus. Reserve((int)ECharaStatus::Num / 32 + 1);
	mCounter.Reserve((int)ECharaStatus::Num);
}

void UTsStatusComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UTsStatusComponent::TickComponent(float df, ELevelTick type, FActorComponentTickFunction* func)
{
	Super::TickComponent(df, type, func);

	// ...

//	for(int 1)
	//E_Poison		UMETA(DisplayName = "Poison"),
	//E_Pararise		UMETA(DisplayName = "Pararise"),
}

void	UTsStatusComponent::Set( ECharaStatus s, float counter )
{
	int idx = (int)s / 32;
	int bit = (int)s % 32;
	if (idx < mStatus.Num()){
		mStatus[idx] |= (1UL << bit);
		mCounter[(int)s] = counter;
	}
}

void	UTsStatusComponent::Sets(const TArray<ECharaStatus>& params, float counter ) 
{
	for (auto s : params) {
		int idx = (int)s / 32;
		int bit = (int)s % 32;

		if (idx >= mStatus.Num()) break;

		mStatus[idx] |= (1UL << bit);
		mCounter[(int)s] = counter;
	}
}

void	UTsStatusComponent::Reset(ECharaStatus s)
{
	int idx = (int)s / 32;
	int bit = (int)s % 32;

	if (idx < mStatus.Num()){ 
		mStatus[idx] &= ~(1UL << bit);
		mCounter[(int)s] = 0;
	}
}

void	UTsStatusComponent::Resets(const TArray<ECharaStatus>& params)
{
	for (auto s : params) {
		int idx = (int)s / 32;
		int bit = (int)s % 32;

		if (idx >= mStatus.Num()) break;

		mStatus[idx] &= ~(1UL << bit);
		mCounter[(int)s] = 0;
	}
}

bool	UTsStatusComponent::Is(ECharaStatus s) const
{
	int idx = (int)s / 32;
	int bit = (int)s % 32;
	if ( idx >= mStatus.Num()) return false;
	return (mStatus[idx] & (1UL << bit)) > 0;
}

bool	UTsStatusComponent::Or(const TArray<ECharaStatus>& params) const
{
	for (auto s : params)
		if (  Is(s) ) return true;
	return false;
}

bool	UTsStatusComponent::And(const TArray<ECharaStatus>& params) const
{
	for (auto s : params)
		if ( !Is(s) ) return false;
	return true;
}
