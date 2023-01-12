// Copyright Epic Games, Inc. All Rights Reserved.

#include "compakt.h"
#include "compaktStyle.h"
#include "compaktCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "UObject/Package.h"
#include <FileHelpers.h>
#include <DesktopPlatform/Public/IDesktopPlatform.h>
#include <DesktopPlatform/Public/DesktopPlatformModule.h>
#include <ContentBrowserModule.h>
#include "IContentBrowserSingleton.h"




static const FName compaktTabName("Pack");

#define LOCTEXT_NAMESPACE "FcompaktModule"

void FcompaktModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FcompaktStyle::Initialize();
	FcompaktStyle::ReloadTextures();

	FcompaktCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FcompaktCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FcompaktModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FcompaktModule::RegisterMenus));
}

void FcompaktModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FcompaktStyle::Shutdown();

	FcompaktCommands::Unregister();
}

void FcompaktModule::PluginButtonClicked()
{



	FString PackTemplateDir = FPaths::ProjectDir() + "Plugins/compakt/Template";
	FString ContentDir = FPaths::ProjectDir() + "Content/";
	FString UnrealPakPath = FPaths::RootDir() + "Engine/Binaries/Win64/UnrealPak.exe";
	FString CompaktTempDir = "C:/temp/compakt/";
	FString CompaktContentDir = "C:/temp/compakt/Content/";

	//UE_LOG(LogTemp, Warning, TEXT("FilePaths: EngineDir: %s"), *PackTemplateDir);


	TArray<FString> Folders;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();
	ContentBrowserSingleton.GetSelectedFolders(Folders);

	// Check if something is selected in the content browser
	if (Folders.Num() > 0) {
		FString selectedFolder = *Folders[0];
		selectedFolder.RemoveAt(0, 6, true);
		UE_LOG(LogTemp, Warning, TEXT("selected folder: %s"), *selectedFolder);
		ContentDir += *selectedFolder;
		CompaktContentDir += *selectedFolder;

		// Save all dirty packages (unsaved assets)
		bool bSaved = FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, true, nullptr);


		IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Delete the temporary folder and copy the new template files and content
		if (platformFile.DeleteDirectoryRecursively(*CompaktTempDir))
		{
			platformFile.CopyDirectoryTree(*CompaktTempDir, *PackTemplateDir, true);
			platformFile.CopyDirectoryTree(*CompaktContentDir, *ContentDir, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Temp directory could not be deleted"));
		}



		// Create save dialog 
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

		const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString Title = "Save UPACK";
		const FString CurrentFilename = *selectedFolder;
		const FString FileTypes = TEXT("Unreal Feature Pack (*.upack)|*.upack");

		TArray<FString> OutFilenames;
		DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			"Save UPACK",
			(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetPath(CurrentFilename),
			(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".upack"),
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);

		if (OutFilenames.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("filenames: %s"), *OutFilenames[0]);
			FString Attributes = "-Create " + OutFilenames[0] + " " + *CompaktTempDir;

			FPlatformProcess::CreateProc
			(
				*UnrealPakPath,
				*Attributes,
				true,
				false,
				false,
				nullptr,
				0,
				nullptr,
				nullptr
			);
		}

		FText DialogText = FText::Format(
			LOCTEXT("PluginButtonDialogText", "Done packing!"),
			FText::FromString(TEXT("FcompaktModule::PluginButtonClicked()")),
			FText::FromString(TEXT("compakt.cpp"))
		);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		

	}
	else {
		FText DialogText = FText::Format(
			LOCTEXT("PluginButtonDialogText", "Please first select a folder in the Content Browser to pack."),
			FText::FromString(TEXT("FcompaktModule::PluginButtonClicked()")),
			FText::FromString(TEXT("compakt.cpp"))
		);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
	}

	

	
}

void FcompaktModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FcompaktCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FcompaktCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

bool SaveFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{

	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bFileChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{

		bFileChosen = DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			Title,
			InOutLastPath,
			DefaultFile,
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);
	}

	bFileChosen = (OutFilenames.Num() > 0);

	if (bFileChosen)
	{

		// User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutFilenames[0];
		OutFilename = OutFilenames[0];
	}

	return bFileChosen;
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FcompaktModule, compakt)