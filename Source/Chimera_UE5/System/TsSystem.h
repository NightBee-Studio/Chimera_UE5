// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Services/TsService.h"

#include "Services/ParamService/TsParam.h"
#include "Services/ParamService/TsParamService.h"

#include "Kernel/TsSingleton.h"
#include "Kernel/TsAppKernel.h"

//enum defines
//#define TsEN(_hd,_v)		E##_hd##_##_v	UMETA(DisplayName = #_v)
#define TsEN(_hd,_v)		E##_v	UMETA(DisplayName = #_v)

#define TsSERVICE(T)				UTsAppKernel::GetService<T>()
#define TsSERVICE_Call(T,method)	if (T*s = UTsAppKernel::GetService<T>()) s->method
