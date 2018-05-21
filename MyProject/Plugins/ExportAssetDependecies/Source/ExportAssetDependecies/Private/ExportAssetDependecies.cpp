// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ExportAssetDependecies.h"
#include "ExportAssetDependeciesStyle.h"
#include "ExportAssetDependeciesCommands.h"

#include "ExportAssetDependeciesSettings.h"
#include "SlateBasics.h"
#include "SlateExtras.h"

#include "LevelEditor.h"

#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "AssetRegistryModule.h"
#include "ARFilter.h"
#include "ModuleManager.h"
#include "ISettingsModule.h"
#include "PlatformFile.h"
#include "PlatformFilemanager.h"
#include "FileHelper.h"
#include "json.h"
//#include "Editor/EditorEngine.h"
#include "Editor.h"


DEFINE_LOG_CATEGORY(LogExportAssetDependecies);
#define LOCTEXT_NAMESPACE "FExportAssetDependeciesModule"

class FPackageContentNotificationTask
{
public:

	FPackageContentNotificationTask(TWeakPtr<SNotificationItem> InNotificationItemPtr, SNotificationItem::ECompletionState InCompletionState, const FText& InText)
		: CompletionState(InCompletionState)
		, NotificationItemPtr(InNotificationItemPtr)
		, Text(InText)
	{ }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		if (NotificationItemPtr.IsValid())
		{
			if (CompletionState == SNotificationItem::CS_Fail)
			{
				GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
			}
			else
			{
				GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
			}

			TSharedPtr<SNotificationItem> NotificationItem = NotificationItemPtr.Pin();
			NotificationItem->SetText(Text);
			NotificationItem->SetCompletionState(CompletionState);
			NotificationItem->ExpireAndFadeout();
		}
	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FPackageContentNotificationTask, STATGROUP_TaskGraphTasks);
	}

private:

	SNotificationItem::ECompletionState CompletionState;
	TWeakPtr<SNotificationItem> NotificationItemPtr;
	FText Text;
};


class FPackageContentCompleteTask
{
public:

	FPackageContentCompleteTask(const FString& InPakFileAbsolutePath)
		: PakFileAbsolutePath(InPakFileAbsolutePath)
	{ }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
//		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//
//		if (DesktopPlatform != nullptr)
//		{
//#if PLATFORM_WINDOWS
//			// Can't use UserDir here as editor may have been run with -installed, UAT will save to GameDir()/Saved
//			FString PakPath = FPaths::ConvertRelativePathToFull(FPaths::GameDir() / TEXT("Saved") / TEXT("StagedBuilds") / DLCName / TEXT("WindowsNoEditor") / TEXT("UnrealTournament") / TEXT("Content") / TEXT("Paks") / DLCName + TEXT("-WindowsNoEditor.pak"));
//
//			// Copy to game directory for now, launcher may do this for us later
//			FString DestinationPath = FPaths::ConvertRelativePathToFull(FString(FPlatformProcess::UserDir()) / FApp::GetGameName() / TEXT("Saved") / TEXT("Paks") / TEXT("MyContent") / DLCName + "-WindowsNoEditor.pak");
//#elif PLATFORM_LINUX
//			FString PakPath = FPaths::ConvertRelativePathToFull(FPaths::GameSavedDir() / TEXT("StagedBuilds") / DLCName / TEXT("LinuxNoEditor") / TEXT("UnrealTournament") / TEXT("Content") / TEXT("Paks") / DLCName + TEXT("-LinuxNoEditor.pak"));
//
//			// Copy to game directory for now, launcher may do this for us later
//			FString DestinationPath = FPaths::ConvertRelativePathToFull(FString(FPlatformProcess::UserDir()) / FApp::GetGameName() / TEXT("Saved") / TEXT("Paks") / TEXT("MyContent") / DLCName + "-LinuxNoEditor.pak");
//#else
//			FString PakPath = FPaths::ConvertRelativePathToFull(FPaths::GameSavedDir() / TEXT("StagedBuilds") / DLCName / TEXT("MacNoEditor") / TEXT("UnrealTournament") / TEXT("Content") / TEXT("Paks") / DLCName + TEXT("-MacNoEditor.pak"));
//
//			// Copy to game directory for now, launcher may do this for us later
//			FString DestinationPath = FPaths::ConvertRelativePathToFull(FString(FPlatformProcess::UserDir()) / FApp::GetGameName() / TEXT("Saved") / TEXT("Paks") / TEXT("MyContent") / DLCName + "-MacNoEditor.pak");
//#endif
//			if (IFileManager::Get().Copy(*DestinationPath, *PakPath, true) != COPY_OK)
//			{
//				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CopyPackageFailed", "Could not copy completed package file to game directory."));
//				return;
//			}
//
//			TSharedPtr<SWindow> MsgWindow = NULL;
//			TSharedPtr<SPackageCompleteChoiceDialog> MsgDialog = NULL;
//
//			MsgWindow = SNew(SWindow)
//				.Title(NSLOCTEXT("PublishContentSuccessTitle", "MessageTitle", "Would you like to share?"))
//				.SizingRule(ESizingRule::Autosized)
//				.AutoCenter(EAutoCenter::PreferredWorkArea)
//				.SupportsMinimize(false).SupportsMaximize(false);
//
//			MsgDialog = SNew(SPackageCompleteChoiceDialog)
//				.ParentWindow(MsgWindow)
//				.Message(LOCTEXT("PublishContentSuccess", "Content packaged successfully. Would you like to share it now?\n\nIf you share this content, it will be uploaded to UT File Storage so it can be shared with your friends if you start a game server using this content."))
//				.WrapMessageAt(512.0f)
//				.FilePath(DestinationPath)
//				.FilePathLinkText(LOCTEXT("PublishContentSuccessOpenWindow", "Open Window To Packaged Content Directory"))
//				.MessageType(EAppMsgType::YesNo);
//
//			MsgDialog->ResultCallback = nullptr;
//
//			MsgWindow->SetContent(MsgDialog.ToSharedRef());
//
//			GEditor->EditorAddModalWindow(MsgWindow.ToSharedRef());
//
//			EAppReturnType::Type Response = MsgDialog->GetResponse();
//
//			if (EAppReturnType::Yes == Response)
//			{
//			//	FString LauncherCommandLine = TEXT("-assetuploadcategory=ut -assetuploadpath=\"") + PakPath + TEXT("\"");
//
//			}
//		}
//		

// UE4 API to show an editor notification.
		auto Message = LOCTEXT("ExportAssetDependeciesSuccessNotification", "Succeed to Package Pak file.");
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 10.0f;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = true;

		const FString HyperLinkText = PakFileAbsolutePath;
		Info.Hyperlink = FSimpleDelegate::CreateStatic([](FString SourceFilePath)
		{
			FPlatformProcess::ExploreFolder(*SourceFilePath);
		}, HyperLinkText);
		Info.HyperlinkText = FText::FromString(HyperLinkText);

		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Success);

	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FPackageContentCompleteTask, STATGROUP_TaskGraphTasks);
	}

private:

	FString PakFileAbsolutePath;
};



static const FName ExportAssetDependeciesTabName("ExportAssetDependecies");



void FExportAssetDependeciesModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

    FExportAssetDependeciesStyle::Initialize();
    FExportAssetDependeciesStyle::ReloadTextures();

    FExportAssetDependeciesCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FExportAssetDependeciesCommands::Get().PluginAction,
        FExecuteAction::CreateRaw(this, &FExportAssetDependeciesModule::PluginButtonClicked),
        FCanExecuteAction());

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    {
        TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
        MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FExportAssetDependeciesModule::AddMenuExtension));

        LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
    }

    {
        TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
        ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FExportAssetDependeciesModule::AddToolbarExtension));

        LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
    }

    // Initialize setting, take care of this registry syntax
    {
        ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
        if (SettingsModule != nullptr)
        {
            // ClassViewer Editor Settings
            SettingsModule->RegisterSettings("Project", "Game", "ExportAssetDependencies",
                LOCTEXT("ExportAssetDependenciesSettingsName", "Export Asset Dependencies"),
                LOCTEXT("ExportAssetDependenciesSettingsDescription", "Export Asset Dependencies."),
                GetMutableDefault<UExportAssetDependeciesSettings>()
            );
        }
    }
}

void FExportAssetDependeciesModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    FExportAssetDependeciesStyle::Shutdown();

    FExportAssetDependeciesCommands::Unregister();
}

void FExportAssetDependeciesModule::PluginButtonClicked()
{
    // TODO ArcEcho
    // Should check whether the game content is dirty.

    // If loading assets
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    if (AssetRegistryModule.Get().IsLoadingAssets())
    {
        // We are still discovering assets, listen for the completion delegate before building the graph
        if (!AssetRegistryModule.Get().OnFilesLoaded().IsBoundToObject(this))
        {
            AssetRegistryModule.Get().OnFilesLoaded().AddRaw(this, &FExportAssetDependeciesModule::ExportAssetDependecies);
        }
    }
    else
    {
        ExportAssetDependecies();
    }
}


void FExportAssetDependeciesModule::AddMenuExtension(FMenuBuilder& Builder)
{
    Builder.AddMenuEntry(FExportAssetDependeciesCommands::Get().PluginAction);
}

struct FDependicesInfo
{
    TArray<FAssetData> DependicesInGameContentDir;
    TArray<FAssetData> OtherDependices;
};

void FExportAssetDependeciesModule::ExportAssetDependecies()
{
	// Validate settings

	TMap<FName, TArray<FFilePath>> PackagesNeedToExportMap = GetPackagesNeedToExport();

	if (PackagesNeedToExportMap.Num() == 0)
	{
		// TODO ArcEcho
		// 1.Check input paths validation
		// 2.Check it has valid package path.
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule != nullptr)
		{
			// If there is no PackagesToExport set, just to the setting viewer.
			SettingsModule->ShowViewer("Project", "Game", "ExportAssetDependencies");

			// UE4 API to show an editor notification.
			auto Message = LOCTEXT("ExportAssetDependeciesNoValidTargetPackages", "No valid target packages set.");
			FNotificationInfo Info(Message);
			Info.bUseSuccessFailIcons = true;
			FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Fail);

			UE_LOG(LogExportAssetDependecies, Log, TEXT("No valid target packages set."));
			return;
		}
	}


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	

	TMap< FName, FString> NamedPakListMap;


		for (TPair<FName, TArray<FFilePath>> NamedPackageFilePathArray : PackagesNeedToExportMap)
		{  // in one named group, asset<--->depend
			const FString TheUniqueGroupName = NamedPackageFilePathArray.Key.ToString();

	

			TMap<FAssetData, FDependicesInfo> DependicesInfos;
			for (FFilePath PackageFilePath : NamedPackageFilePathArray.Value)
			{
				FStringAssetReference AssetRef = PackageFilePath.FilePath;
				FString TargetLongPackageName = AssetRef.GetLongPackageName();

				FString OutFileName;
				if (FPackageName::DoesPackageExist(TargetLongPackageName, nullptr, &OutFileName))
				{
					auto  AssetDataList = FindFirstAssetDataByLongPackageName(AssetRegistryModule, TargetLongPackageName);
					if (ensure(AssetDataList.Num() > 0))
					{
						FDependicesInfo& 	DependicesInfo = DependicesInfos.FindOrAdd(AssetDataList[0]);
						GatherDependenciesInfoRecursively(AssetRegistryModule, AssetDataList[0], DependicesInfo.DependicesInGameContentDir, DependicesInfo.OtherDependices);
					}
				}
			}
			///
			TArray<FAssetData> PackageNeedBePak;
			for (TPair<FAssetData, FDependicesInfo> Pair : DependicesInfos)
			{
				PackageNeedBePak.AddUnique(Pair.Key);

				for (FAssetData Dep : Pair.Value.DependicesInGameContentDir)
				{
					PackageNeedBePak.AddUnique(Dep);
				}
			}
			TArray<FString> AbsoluteFilePaths;
			for (FAssetData OnePackage : PackageNeedBePak)
			{
				FString UsedRelativeCookPackagePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Cooked"), TEXT("WindowsNoEditor"), FApp::GetProjectName(), TEXT("Content"),
					OnePackage.PackagePath.ToString().Replace(TEXT("/Game"), TEXT("")));

				/* all files in this dictionary */
				TArray<FString> FoundFiles;
				IFileManager::Get().FindFiles(FoundFiles, *UsedRelativeCookPackagePath);
				for (FString FileName : FoundFiles)
				{ /* only need name like AssetName.* (AssetName.uasset,AssetName.uexp) */
					if (FileName.Contains(OnePackage.AssetName.ToString() + TEXT(".")))
					{
						FString RelativeFilePath = FPaths::Combine(UsedRelativeCookPackagePath, FileName);
						AbsoluteFilePaths.AddUnique(FPaths::ConvertRelativePathToFull(RelativeFilePath));
					}
				}
			}

			TArray<FString> PakSecondPartPaths;
			// paklist's line need to be "D:\~\Cooked\WindowsNoEditor\MyProject\Content\MyProject\Materials\M_Test_01.uexp" "../../../MyProject/Content/MyProject/Materials/M_Test_01.uexp"
			for (const FString& AbsPath : AbsoluteFilePaths)
			{
				const static FString FromReplace = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Cooked"), TEXT("WindowsNoEditor")));

				PakSecondPartPaths.AddUnique(AbsPath.Replace(*FromReplace, TEXT("../../..")));
			}

			const FString PakListName = FString::Printf(TEXT("%s.txt"),*TheUniqueGroupName);

			FString MyPakListFileName = FPaths::Combine(FPaths::ProjectSavedDir(), *PakListName);
			TUniquePtr<FArchive> PakListFileWriter(IFileManager::Get().CreateFileWriter(*MyPakListFileName));

			check(AbsoluteFilePaths.Num() == PakSecondPartPaths.Num());

			for (int32 i = 0; i < AbsoluteFilePaths.Num(); i++)
			{
				FString FirstPart = AbsoluteFilePaths[i].Replace(TEXT("/"), TEXT("\\"));
				FString SecondPart = PakSecondPartPaths[i];
				FString PakListLine = FString::Printf(TEXT(R"("%s" "%s")"), *FirstPart, *SecondPart) + TEXT("\r\n");
				// PakListLine = "FirstPart" "SecondPart" + TEXT("\r\n") 
				PakListFileWriter->Serialize(TCHAR_TO_ANSI(*PakListLine), PakListLine.Len());
			}
			PakListFileWriter->Close();
			const FString AbslutePakListFileName = FPaths::ConvertRelativePathToFull(MyPakListFileName);

///
			{
				// UE4 API to show an editor notification.
				auto Message = LOCTEXT("ExportAssetDependeciesSuccessNotification", "Succeed to export PakList.");
				FNotificationInfo Info(Message);
				Info.bFireAndForget = true;
				Info.ExpireDuration = 5.0f;
				Info.bUseSuccessFailIcons = false;
				Info.bUseLargeFont = false;

				const FString HyperLinkText = AbslutePakListFileName;
				Info.Hyperlink = FSimpleDelegate::CreateStatic([](FString SourceFilePath)
				{
					FPlatformProcess::ExploreFolder(*SourceFilePath);
				}, HyperLinkText);
				Info.HyperlinkText = FText::FromString(HyperLinkText);

				FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Success);
			}
			FString UnrealPakPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir() / TEXT("Binaries/Win64/UnrealPak.exe"));


			const FString PakFileName = FString::Printf(TEXT("%s.pak"), *TheUniqueGroupName);

			FString OutPutFilePath = TEXT(R"(E:\)")+ PakFileName;

			FString PakCommandLine = OutPutFilePath + FString(TEXT(R"( -compress -Create=)")) + FString(TEXT(R"(")")) + AbslutePakListFileName + FString(TEXT(R"(")"));

	//		FString ListPakCommandLine = OutPutFilePath + TEXT(" -list");

			RunMonitoredProcess(UnrealPakPath,
				PakCommandLine,
				FText::FromString(TEXT("Process Pak")),
				FText::FromString("Process Pak"),
				FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
				OutPutFilePath
			);

		}

	






	
	




	/*RunMonitoredProcess(UnrealPakPath,
		ListPakCommandLine,
		FText::FromString(TEXT("List Pak")),
		FText::FromString("List Pak"),
		FEditorStyle::GetBrush(TEXT("MainFrame.CookContent"))
	);*/

}

    




TArray<FAssetData> FExportAssetDependeciesModule::FindFirstAssetDataByLongPackageName(FAssetRegistryModule &AssetRegistryModule,FString TargetLongPackageName)
{
	TArray<FAssetData> AssetDataList;

	bool  bResult = AssetRegistryModule.Get().GetAssetsByPackageName(FName(*TargetLongPackageName), AssetDataList);
	if (!bResult || AssetDataList.Num() == 0)
	{
		UE_LOG(LogExportAssetDependecies, Error, TEXT("Failed to get AssetData of  %s, please check."), *TargetLongPackageName);
	}

	if (AssetDataList.Num() > 1)
	{
		UE_LOG(LogExportAssetDependecies, Error, TEXT("Got multiple AssetData of  %s, please check."), *TargetLongPackageName);
	}

	return AssetDataList;
}

void FExportAssetDependeciesModule::GatherDependenciesInfoRecursively(FAssetRegistryModule &AssetRegistryModule, const FAssetData &TargetAssetData, TArray<FAssetData> &DependicesInGameContentDir, TArray<FAssetData> &OtherDependices)
{
	TArray<FName> DependencedPackageNames;

	bool bGetDependenciesSuccess = AssetRegistryModule.Get().GetDependencies(TargetAssetData.PackageName, DependencedPackageNames, EAssetRegistryDependencyType::Packages);

	if (!bGetDependenciesSuccess)
	{
		return;
	}

	for (auto &TestSingleName : DependencedPackageNames)
	{
		// Pick out packages in game content dir.
		FString LongDependentPackageName = TestSingleName.ToString();
		auto  FoundAssets = FindFirstAssetDataByLongPackageName(AssetRegistryModule,LongDependentPackageName);

		if (FoundAssets.Num() == 0)
		{
			continue;
		}

		const FAssetData& FoundAsset = FoundAssets[0];

		if (LongDependentPackageName.StartsWith("/Game"))
		{
			if (DependicesInGameContentDir.Find(FoundAsset) == INDEX_NONE)
			{
				DependicesInGameContentDir.Add(FoundAsset);
				GatherDependenciesInfoRecursively(AssetRegistryModule, FoundAsset, DependicesInGameContentDir, OtherDependices);
			}

		}
		else
		{
			if (OtherDependices.Find(FoundAsset) == INDEX_NONE)
			{
				OtherDependices.Add(FoundAsset);
				GatherDependenciesInfoRecursively(AssetRegistryModule, FoundAsset, DependicesInGameContentDir, OtherDependices);
			}
		}
	}

	
}


void FExportAssetDependeciesModule::RunMonitoredProcess(const FString& OutExePath, const FString& CommandLine ,const FText& TaskName, const FText &TaskShortName, const FSlateBrush* TaskIcon, FString DesiredFileOutputPath /*= TEXT("")*/)
{
	FString CmdExe = TEXT("cmd.exe");

	FString FullCommandLine = FString::Printf(TEXT("/c \"\"%s\" %s\""), *OutExePath, *CommandLine);

	UE_LOG(LogTemp, Log, TEXT("Starting RunMonitoredProcess Task:%s %s"), *CmdExe, *FullCommandLine);


	TSharedPtr<FMonitoredProcess> UatProcess = MakeShareable(new FMonitoredProcess(CmdExe, FullCommandLine, true));

	// create notification item
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("TaskName"), TaskName);
	FNotificationInfo Info(FText::Format(LOCTEXT("UatTaskInProgressNotification", "{TaskName}..."), Arguments));

	Info.Image = TaskIcon;
	Info.bFireAndForget = false;
	Info.ExpireDuration = 3.0f;
	Info.Hyperlink = FSimpleDelegate::CreateStatic(&FExportAssetDependeciesModule::HandleUatHyperlinkNavigate);
	Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
	Info.ButtonDetails.Add(
		FNotificationButtonInfo(
		LOCTEXT("UatTaskCancel", "Cancel"),
		LOCTEXT("UatTaskCancelToolTip", "Cancels execution of this task."),
		FSimpleDelegate::CreateStatic(&FExportAssetDependeciesModule::HandleUatCancelButtonClicked, UatProcess)
		)
		);

	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

	if (!NotificationItem.IsValid())
	{
		return;
	}
	
	NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);

	// launch the packager
	TWeakPtr<SNotificationItem> NotificationItemPtr(NotificationItem);

	UatProcess->OnCanceled().BindStatic(&FExportAssetDependeciesModule::HandleMonitoredProcessCanceled, NotificationItemPtr, TaskShortName);
	UatProcess->OnCompleted().BindStatic(&FExportAssetDependeciesModule::HandleMonitoredProcessCompleted, NotificationItemPtr, TaskShortName, DesiredFileOutputPath);
	UatProcess->OnOutput().BindStatic(&FExportAssetDependeciesModule::HandleMonitoredProcessOutput, NotificationItemPtr, TaskShortName);

	if (UatProcess->Launch())
	{
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	}
	else
	{
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));

		NotificationItem->SetText(LOCTEXT("UatLaunchFailedNotification", "Failed to launch Unreal Automation Tool (UAT)!"));
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		NotificationItem->ExpireAndFadeout();
	}

}

void FExportAssetDependeciesModule::HandleMonitoredProcessCanceled(TWeakPtr<SNotificationItem> NotificationItemPtr,  FText TaskName)
{
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("TaskName"), TaskName);

	TGraphTask<FPackageContentNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
		NotificationItemPtr,
		SNotificationItem::CS_Fail,
		FText::Format(LOCTEXT("UatProcessFailedNotification", "{TaskName} canceled!"), Arguments)
	);
}

void FExportAssetDependeciesModule::HandleMonitoredProcessCompleted(int32 ReturnCode, TWeakPtr<SNotificationItem> NotificationItemPtr,  FText TaskName, FString DesiredOpenFileOutputPath)
{
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("TaskName"), TaskName);

	if (ReturnCode == 0)
	{
		TGraphTask<FPackageContentNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Success,
			FText::Format(LOCTEXT("UatProcessSucceededNotification", "{TaskName} complete!"), Arguments)
		);

		// Run this on the main thread

		if (!DesiredOpenFileOutputPath.IsEmpty())
		{
			TGraphTask<FPackageContentCompleteTask>::CreateTask().ConstructAndDispatchWhenReady(DesiredOpenFileOutputPath);
		}

		
	}
	else
	{
		TGraphTask<FPackageContentNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Fail,
			FText::Format(LOCTEXT("PackagerFailedNotification", "{TaskName} failed!"), Arguments)
		);
	}
}

void FExportAssetDependeciesModule::HandleMonitoredProcessOutput(FString Output,TWeakPtr<SNotificationItem> NotificationItemPtr,  FText TaskName)
{
	if (!Output.IsEmpty() && !Output.Equals("\r"))
	{
		UE_LOG(LogExportAssetDependecies, Log, TEXT("%s: %s"), *TaskName.ToString(), *Output);
	}
}

void FExportAssetDependeciesModule::HandleUatHyperlinkNavigate()
{
	FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog"));
}

void FExportAssetDependeciesModule::HandleUatCancelButtonClicked(TSharedPtr<FMonitoredProcess> PackagerProcess)
{
	if (PackagerProcess.IsValid())
	{
		PackagerProcess->Cancel(true);
	}
}

TMap<FName, TArray<FFilePath>> FExportAssetDependeciesModule::GetPackagesNeedToExport() const
{
	TMap<FName, TArray<FFilePath>> Results;

	const UExportAssetDependeciesSettings* CurrentSettings = GetDefault<UExportAssetDependeciesSettings>();
	if (!CurrentSettings)
	{
		UE_LOG(LogExportAssetDependecies, Error, TEXT("Cannot read ExportAssetDependeciesSettings"));
		return Results;
	}

	for (TPair<FName,FOnePakInfo> Pair : CurrentSettings->PackagesToExportMap)
	{
		const  TArray<FFilePath>& PackagesToExport = Pair.Value.PackagesToExport;

		for (const FFilePath& FilePath : PackagesToExport)
		{
			Results.FindOrAdd(Pair.Key).AddUnique(FilePath);
		}

	}

	return Results;
}

//void FExportAssetDependeciesModule::SaveDependicesInfo(const TMap<FString, FDependicesInfo> &DependicesInfos)
//{
//    TSharedPtr<FJsonObject> RootJsonObject = MakeShareable(new FJsonObject);
//    for (auto &DependicesInfoEntry : DependicesInfos)
//    {
//        TSharedPtr<FJsonObject> EntryJsonObject = MakeShareable(new FJsonObject);
//
//        // Write current AssetClass.
//        EntryJsonObject->SetStringField("AssetClass", DependicesInfoEntry.Value.AssetClassString);
//
//        // Write dependencies in game content dir.
//        {
//            TArray< TSharedPtr<FJsonValue> > DependenciesEntry;
//            for (auto &d : DependicesInfoEntry.Value.DependicesInGameContentDir)
//            {
//                DependenciesEntry.Add(MakeShareable(new FJsonValueString(d)));
//            }
//            EntryJsonObject->SetArrayField("DependenciesInGameContentDir", DependenciesEntry);
//        }
//
//        // Write dependencies not in game content dir.
//        {
//            TArray< TSharedPtr<FJsonValue> > DependenciesEntry;
//            for (auto &d : DependicesInfoEntry.Value.OtherDependices)
//            {
//                DependenciesEntry.Add(MakeShareable(new FJsonValueString(d)));
//            }
//            EntryJsonObject->SetArrayField("OtherDependencies", DependenciesEntry);
//        }
//
//        RootJsonObject->SetObjectField(DependicesInfoEntry.Key, EntryJsonObject);
//    }
//
//    FString OutputString;
//    auto JsonWirter = TJsonWriterFactory<>::Create(&OutputString);
//    FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), JsonWirter);
//
//    FString ResultFileFilename = FPaths::Combine(FPaths::GameSavedDir(), TEXT("ExportAssetDependecies"), TEXT("/AssetDependencies.json"));
//    ResultFileFilename = FPaths::ConvertRelativePathToFull(ResultFileFilename);
//
//    // Attention to FFileHelper::EEncodingOptions::ForceUTF8 here. 
//    // In some case, UE4 will save as UTF16 according to the content.
//    bool bSaveSuccess = FFileHelper::SaveStringToFile(OutputString, *ResultFileFilename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
//    if (bSaveSuccess)
//    {
//        // UE4 API to show an editor notification.
//        auto Message = LOCTEXT("ExportAssetDependeciesSuccessNotification", "Succeed to export asset dependecies.");
//        FNotificationInfo Info(Message);
//        Info.bFireAndForget = true;
//        Info.ExpireDuration = 5.0f;
//        Info.bUseSuccessFailIcons = false;
//        Info.bUseLargeFont = false;
//
//        const FString HyperLinkText = ResultFileFilename;
//        Info.Hyperlink = FSimpleDelegate::CreateStatic([](FString SourceFilePath)
//        {
//            FPlatformProcess::ExploreFolder(*SourceFilePath);
//        }, HyperLinkText);
//        Info.HyperlinkText = FText::FromString(HyperLinkText);
//
//        FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Success);
//
//        UE_LOG(LogExportAssetDependecies, Log, TEXT("%s. At %s"), *Message.ToString(), *ResultFileFilename);
//    }
//    else
//    {
//        UE_LOG(LogExportAssetDependecies, Error, TEXT("Failed to export %s"), *ResultFileFilename);
//    }
//}

void FExportAssetDependeciesModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
    Builder.AddToolBarButton(FExportAssetDependeciesCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FExportAssetDependeciesModule, ExportAssetDependecies)