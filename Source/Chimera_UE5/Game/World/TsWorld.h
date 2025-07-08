#pragma once

#include "CoreMinimal.h"

#include "TsWorld.generated.h"


// -------------------------------- UTsWorld  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API ATsWorld
	: public AActor
{
	GENERATED_BODY()

private:


protected:
	virtual void BeginPlay() override{;}

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Meta = (RowType = "FBiomeSpec"))
	TObjectPtr<UDataTable>		mBiomeSpecs;

	AActor*		SpawnObject(TSubclassOf<AActor> cls, const FString& name, const FVector& pos, const FRotator& rot);

	virtual void Tick(float dt) override { ; }
};

