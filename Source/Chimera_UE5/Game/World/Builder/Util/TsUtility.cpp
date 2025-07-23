#include "TsUtility.h"
#include "CoreMinimal.h"



void	TsUtil::RandSeed(int i) {
	FMath::RandInit( i );
}

int32	TsUtil::RandRange(int32 a, int32 b)
{
	return FMath::RandRange(a, b);
}

float	TsUtil::RandRange(float a, float b) {
	return FMath::RandRange(a, b);
}

float	TsUtil::RandRange(const FVector2D &v){
	return FMath::RandRange(v.X, v.Y);
}


static FString gDirectory;
static FString gPackagePath;

#define PROJPATH	"D:\\Works\\Projects\\Chimera\\01_Project\\Chimera_UE5\\Content\\"
#define GAMEPATH	"/Game/"

void				TsUtil::SetDirectory(const FString& path, int no_x, int no_y)
{
	FString fullpath = path ;
	if (no_x >= 0 && no_y >= 0) {
		fullpath += FString::Printf(TEXT("%02d_%02d"), no_x, no_y);
	}
	fullpath += FString("/");

	gDirectory   = PROJPATH + fullpath.Replace(TEXT("/"), TEXT("\\"));
	gPackagePath = GAMEPATH + fullpath ;

	IPlatformFile& pf = FPlatformFileManager::Get().GetPlatformFile();
	if (!pf.DirectoryExists(*gDirectory)) {		// Directory Exists?
		pf.CreateDirectory(*gDirectory);
	}
}

FString TsUtil::GetDirectory(const FString& name)
{
	FString fullpath = gDirectory + name.Replace(TEXT("/"), TEXT("\\"));
	IPlatformFile& pf = FPlatformFileManager::Get().GetPlatformFile();
	if (!pf.DirectoryExists(*fullpath)) {		// Directory Exists?
		pf.CreateDirectory(*fullpath);
	}
	return fullpath ;
}

FString 	TsUtil::GetPackagePath(const FString& name)
{
	return gPackagePath + name ;
}
