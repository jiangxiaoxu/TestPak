#pragma once

#include "ExportAssetDependecies.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "ExportAssetDependeciesSettings.generated.h"

/**
* Settings for export asset dependencies.
*/
USTRUCT(BlueprintType)
struct FOnePakInfo 
{
	GENERATED_BODY()
public:
	/** You can use copied asset string reference here, e.g. World'/Game/NewMap.NewMap'*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		TArray<FFilePath> PackagesToExport;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Default)
		TArray<FDirectoryPath> DirectoryToExport;
};


UCLASS(config = Game, defaultconfig)
class EXPORTASSETDEPENDECIES_API UExportAssetDependeciesSettings : public UObject
{
    GENERATED_BODY()

public:
	/* key will be .Pak file name  */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = Default)
		TMap<FName, FOnePakInfo>  PackagesToExportMap;
};
