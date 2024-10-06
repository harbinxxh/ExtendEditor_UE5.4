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

	//此函数的作用是定义要插入菜单项的位置
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	//设置自定义菜单项的详细信息例如，标题、提示信息、和执行的函数
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	//菜单项需要执行的函数-删除未使用的资产
	void OnDeleteUnusedAssetButtonClicked();

#pragma endregion

};
