// Fill out your copyright notice in the Description page of Project Settings.


#include "TsUIService.h"
#include "System/Kernel/TsAppKernel.h"
#include "System/Services/LocalizeService/TsLocalizeService.h"



int	UTsWidget::gID = 0 ;

UTsWidget::UTsWidget(const FObjectInitializer& obj_init)
	: Super(obj_init)
{
}

#define DOSEVICE(T,method)	if ( T *srv = UTsAppKernel::GetService<T>()) srv->method

UTsWidget::~UTsWidget()
{
	TsSERVICE_Call(UTsUIService,Remove(this));
}

bool UTsWidget::Initialize()
{
	if (Super::Initialize() ) {
		mID = ++gID;

		TsSERVICE_Call(UTsUIService,Add(this));

		return true;
	}
	return false;
}


FText	UTsWidget::GetLocalizedText(const FName &key) const
{
	UTsLocalizeService* srv = TsSERVICE(UTsLocalizeService);
	return srv ? srv->GetText(key) : FText::GetEmpty() ;
}


void	UTsUIService::Initialize(USubsystem* owner, FSubsystemCollectionBase& collection)
{
	Super::Initialize(owner, collection);
}

void	UTsUIService::Deinitialize() 
{
	Super::Deinitialize();
}

UTsWidget*	UTsUIService::Get(TsHandler id)
{
	return nullptr;
}

TsHandler	UTsUIService::Open(EUIID ui)
{
	return 0;
}

void		UTsUIService::Close(EUIID ui)
{
}
