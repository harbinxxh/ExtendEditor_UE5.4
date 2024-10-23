// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SUPERMANAGER_API SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	//用于保存资产数据
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;

	//用于生成列表视图中每一行
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
};
