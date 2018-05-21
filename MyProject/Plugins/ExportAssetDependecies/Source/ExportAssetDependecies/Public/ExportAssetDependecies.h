// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "AssetData.h"

class FToolBarBuilder;
class FMenuBuilder;
class UExportAssetDependeciesSettings;
class FAssetRegistryModule;
struct FDependicesInfo;

DECLARE_LOG_CATEGORY_EXTERN(LogExportAssetDependecies, Log, All);

class FExportAssetDependeciesModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** This function will be bound to Command. */
    void PluginButtonClicked();

private:

    void AddToolbarExtension(FToolBarBuilder& Builder);
    void AddMenuExtension(FMenuBuilder& Builder);

    void ExportAssetDependecies();

	TArray<FAssetData> FindFirstAssetDataByLongPackageName(FAssetRegistryModule &AssetRegistryModule,FString TargetLongPackageName);
	

    void GatherDependenciesInfoRecursively(FAssetRegistryModule &AssetRegistryModule, const FAssetData &TargetAssetData,
        TArray<FAssetData> &DependicesInGameContentDir, TArray<FAssetData> &OtherDependices);

    /** This will save the dependencies information to the OutputPath/AssetDependencies.json */
   // void SaveDependicesInfo(const TMap<FString, FDependicesInfo> &DependicesInfos);

	void RunMonitoredProcess();

private:
    TSharedPtr<class FUICommandList> PluginCommands;
};