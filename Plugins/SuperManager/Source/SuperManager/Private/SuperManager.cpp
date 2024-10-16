// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "ObjectTools.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"


#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ContentBrowserMenuExtention

//初始化自定义菜单
void FSuperManagerModule::InitCBMenuExtention()
{
	//1、动态加载 ContentBrowser 模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//2、获取内容浏览器菜单代理扩展器对象的引用
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//3、创建我们自己定制的菜单代理
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;

	//第一种方式：这种方式更容易理解
	//4、要使自定义代理发挥作用，需要为代理绑定我们自己的成员函数
	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);

	//5、将我们自己的菜单代理添加到CB菜单代理扩展器对象中
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);

	//第二种方式：通过 CreateRaw() 函数，为代理绑定函数
	//ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

//此函数的作用是定义要插入菜单项的位置
TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"), //第一个参数：ExtensionHook，这就是我们想要插入菜单项的位置。
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
			);
	}

	FolderPathsSelected = SelectedPaths;

	return MenuExtender;
}

//设置自定义菜单项的详细信息例如，标题、提示信息、和执行的函数
void FSuperManagerModule::AddCBMenuEntry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete UNused Assets")),
		FText::FromString(TEXT("Safely delete  all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete  all empty folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advance Deletion")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

//菜单项需要执行的函数-删除未使用的资产
void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder!"));
		return;
	}

	// 获取当前选择文件夹中的资产路径列表
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder!"));
		return;
	}

	//如果找到资产数据，要再次显示消息对话框
	EAppReturnType::Type ConfirmResult =
	DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\n Woudle you like to processed?"));

	if (ConfirmResult == EAppReturnType::No) return;

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName:AssetsPathNames)
	{
		if (AssetPathName.Contains("Developers") || AssetPathName.Contains("Collections"))
		{
			continue;
		}

		// 检测目录是否存在
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		// 查找资产的引用
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		// 等于 0，表示此资产未被引用可以删除
		if (AssetReferences.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under  selected folder"));
	}
}

// 删除空文件夹
void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	// 修复资产引用重定向
	FixUpRedirectors();

	// 获取资产列表
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	// 通过循环找出空的目录
	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		// 查询此目录是否存在
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		// 查询目录是否存在资产
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	// 弹出对话框确认是否要删除空文件夹
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folder found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	// 执行删除空的文件夹
	for (const FString& EmptyFoldersPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFoldersPath))
		{
			++Counter;
		}
		else
		{
			DebugHeader::Print(TEXT("Failed to delete " + EmptyFoldersPath), FColor::Red);
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folder"));
	}
}

// 增强删除功能
void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	// 从菜单项中调用选项卡
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::FixUpRedirectors()
{
	//创建重定向对象数组
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	//需要使用模块管理器中 FModuleManager::LoadModuleChecked<>() 函数，动态加载 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//设置过滤器
	FARFilter Filter;
	Filter.bRecursiveClasses = true; //将递归路径设置为 true，这将允许进入所有子文件夹。
	Filter.PackagePaths.Emplace("/Game");//过滤器过滤的文件夹路径
	Filter.ClassPaths.Emplace("ObjectRedirector");//需要指定要过滤的类的名称

	//将包含通过过滤器所筛选出来的资产数据(FAssetData)
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		//需要将 FAssetData 数据格式，转换成 UObjectRedirector 数据格式
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	//需要调用【AssetToolsModule】模块中的【FixUpRefrencers()】函数，这是用来修复重定向器
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#pragma endregion

#pragma region CustomEditorTab

// 注册自定义 Tab 选项卡
void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	/*
	* 1、TabId 选项卡ID ：当调用选项卡时需要此ID
	* 2、FOnSpawnTab 代理，用于生成选项卡的回调函数
	* 3、为选项卡设置显示名称
	* 4、代理回调函数 OnSpawnAdvanceDeletionTab() 生成选项卡
	*/
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"), FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

// 代理回调函数生成选项卡
// 注册时不会立即执行此方法，当需要显示DockTab的时候才会被调用
TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
	SNew(SDockTab).TabRole(ETabRole::NomadTab);
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)