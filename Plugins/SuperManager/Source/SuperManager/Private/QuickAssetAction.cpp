// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		//Print(TEXT("Please enter a VALID number"), FColor::Cyan);
		ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData: SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			//const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString SourceAssetPath = SelectedAssetData.GetSoftObjectPath().ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		//Print(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);
		ShowNotifyInfo(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + " files"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject:SelectedObjects)
	{
		if (!SelectedObject) continue;
		
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();

		if (OldName.StartsWith(*PrefixFound))
		{
			Print(TEXT("Already has prefix added "), FColor::Red);
			continue;
		}

		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;
		
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		++Counter;
	}

	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully Rename ") + FString::FromInt(Counter) + " assets");
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnsedAssetsData;

	FixUpRedirectors();

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetRefrencers = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetSoftObjectPath().ToString());

		if (AssetRefrencers.Num() == 0)
		{
			UnsedAssetsData.Add(SelectedAssetData);
		}
	}

	if (UnsedAssetsData.Num() == 0)
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("No unsed asset found among selected assets"), false);
		return;
	}

	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnsedAssetsData);

	if (NumOfAssetsDeleted == 0) return;
	
	ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}

void UQuickAssetAction::FixUpRedirectors()
{
	//创建重定向对象数组
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	//需要使用模块管理器中 FModuleManager::LoadModuleChecked<>() 函数，动态加载 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//设置过滤器
	FARFilter Filter;
	Filter.bRecursiveClasses = true; //将递归路径设置为 true，这将允许进入所有子文件夹。
	Filter.PackagePaths.Emplace("/Game");//过滤器过滤的文件夹路径
	
	//Filter.ClassNames.Emplace("ObjectRedirector");//
	Filter.ClassPaths.Emplace("ObjectRedirector");//需要指定要过滤的类的名称
	/* 
	* warning C4996 : 'FARFilter::ClassNames' : Class names are now represented by path names.Please use ClassPaths.
	* Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.
	*/

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
