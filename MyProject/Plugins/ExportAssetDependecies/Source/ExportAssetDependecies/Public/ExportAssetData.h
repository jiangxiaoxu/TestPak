// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExportAssetData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FOnePakInfo
{
	GENERATED_BODY()
public:
	/** You can use copied asset string reference here, e.g. World'/Game/NewMap.NewMap'*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		TArray<FFilePath> PackagesToExport;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		TArray<FDirectoryPath> DirectoryToExport;
};


UCLASS()
class  UExportAssetData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/* key will be .Pak file name  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		TMap<FName, FOnePakInfo>  PackagesToExportMap;
	
	
};
