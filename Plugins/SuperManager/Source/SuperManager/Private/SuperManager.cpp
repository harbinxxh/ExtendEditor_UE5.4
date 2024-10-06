// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"

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

	//第一个参数：ExtensionHook，这就是我们想要插入菜单项的位置，但我们如何知道是哪一个位置呢？我们需要回到编辑器中。

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
			);
	}


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

}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)