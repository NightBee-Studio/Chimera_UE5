// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "GameCamera.generated.h"

/**
 * 
 */
UCLASS()
class CHIMERA_UE5_API AGameCamera : public ACameraActor
{
	GENERATED_BODY()

private:
	float				mLength;
	TObjectPtr<APawn>	mTarget;
	FVector2D			mView;

public:
	UPROPERTY(BlueprintReadWrite, Category = "GameCamera")
	FVector2D			mViewMin;
	UPROPERTY(BlueprintReadWrite, Category = "GameCamera")
	FVector2D			mViewMax;

	//TObjectPtr<APawn>	mSpComponent;

protected:
	AGameCamera();

	// Pawn Controlling
public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	virtual void SetTarget( APawn *chara );

	UFUNCTION(BlueprintCallable)
	virtual void SetView(const FVector2D& dir);

	UFUNCTION(BlueprintCallable)
	virtual void SetLength(float leng);
};
