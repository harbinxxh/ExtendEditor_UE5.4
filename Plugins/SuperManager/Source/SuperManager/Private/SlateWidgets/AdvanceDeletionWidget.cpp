// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SlateBasics.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true; // 支持键盘焦点

	StoredAssetsData = InArgs._AssetsDataToStore;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

	ChildSlot
	[
		//Main vertical box
		SNew(SVerticalBox)

		//First vertical slot for title text
		+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("Advance Deletion")))
					.Font(TitleTextFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FColor::White)
			]

		//Second Slot for drop down to specify the listing condition and help text 下拉列表框和资产类的名称
		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
			]

		//Third slot for the asset list 获取资产列表
		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SNew(SListView<TSharedPtr<FAssetData>>)//创建列表视图
							.ItemHeight(24.f)
							.ListItemsSource(&StoredAssetsData) //指定列表数据源
							.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList)//生成列表中的每一行数据
					]
			]

		//Fourth slot for 3 button 按钮点击
		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
			]

	];
}

//用于生成列表视图中每一行
TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	TSharedRef< STableRow< TSharedPtr<FAssetData> >> ListViewRowWidget =
		SNew(STableRow< TSharedPtr<FAssetData> >, OwnerTable).Padding(FMargin(5.f)) //行之间的间隔
		[
			SNew(STextBlock)
				.Text(FText::FromString(DisplayAssetName))
		];

	return ListViewRowWidget;
}
