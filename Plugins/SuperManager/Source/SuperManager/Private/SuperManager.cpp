// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "ObjectTools.h"
#include "EditorAssetLibrary.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();
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

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)