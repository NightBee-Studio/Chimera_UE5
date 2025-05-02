
#include "TsWorld.h"


AActor* ATsWorld::SpawnObject(TSubclassOf<AActor> cls, const FString& name, const FVector& pos, const FRotator & rot )
{
    AActor* actor = nullptr;
    UWorld* world = GetWorld();
    if ( HasAuthority() && world ){
        FActorSpawnParameters param;
        param.Owner = this;
        param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        //FVector     pos = GetActorLocation() + FVector(100, 0, 0);
        //FRotator    rot = FRotator::ZeroRotator;
//
//        actor = world->SpawnActor(cls, pos, rot, param);
//#if WITH_EDITOR
//        if (actor && GIsEditor ){
//            actor->SetFolderPath("World"); // Outliner ��́uworld�v�t�H���_�ɓ����
//            actor->SetActorLabel( name  ) ;
//        }
//#endif
    }
    return actor;
}
