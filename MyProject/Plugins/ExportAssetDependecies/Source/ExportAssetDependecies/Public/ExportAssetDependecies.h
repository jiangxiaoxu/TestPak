// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "AssetData.h"
#include "Misc/MonitoredProcess.h"
#include "EditorStyleSet.h"
#include "Styling/SlateBrush.h"

class FToolBarBuilder;
class FMenuBuilder;
class UExportAssetDependeciesSettings;
class FAssetRegistryModule;
struct FDependicesInfo;


struct FSlateBrush;
class SNotificationItem;

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

	void RunMonitoredProcess(const FString& OutExePath, const FString& CommandLine, const FText& TaskName, const FText &TaskShortName, const struct FSlateBrush* TaskIcon, FString DesiredFileOutputPath=TEXT(""));

	static void HandleMonitoredProcessCanceled(TWeakPtr<SNotificationItem> NotificationItemPtr,  FText TaskShortName);
    
	static void HandleMonitoredProcessCompleted(int32 ReturnCode, TWeakPtr<SNotificationItem> NotificationItemPtr,  FText TaskName,FString DesiredOpenFileOutputPath= TEXT(""));

	static void HandleMonitoredProcessOutput(FString Output,TWeakPtr<SNotificationItem> NotificationItemPtr,  FText TaskShortName);
    
	static void HandleUatHyperlinkNavigate();

	static void HandleUatCancelButtonClicked(TSharedPtr<FMonitoredProcess> PackagerProcess);

	TMap<FName, TArray<FFilePath>> GetPackagesNeedToExport()const;
private:
    TSharedPtr<class FUICommandList> PluginCommands;
};



FORCEINLINE bool operator==(const FFilePath& A, const FFilePath& B)
{
	return A.FilePath == B.FilePath;
}