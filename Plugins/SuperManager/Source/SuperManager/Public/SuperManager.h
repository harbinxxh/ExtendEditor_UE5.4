// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtention

	//初始化自定义菜单
	void InitCBMenuExtention();

	// 保存用户当前选择的文件夹
	TArray<FString> FolderPathsSelected;

	//此函数的作用是定义要插入菜单项的位置
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	//设置自定义菜单项的详细信息例如，标题、提示信息、和执行的函数
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	//菜单项需要执行的函数-删除未使用的资产
	void OnDeleteUnusedAssetButtonClicked();

	// 删除空文件夹
	void OnDeleteEmptyFoldersButtonClicked();

	// 增强删除功能
	void OnAdvanceDeletionButtonClicked();

	// 修复资产引用重定向
	void FixUpRedirectors();

#pragma endregion

#pragma region CustomEditorTab

	// 注册自定义 Tab 选项卡
	void RegisterAdvanceDeletionTab();

	// 代理回调函数生成选项卡
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

#pragma endregion

};
