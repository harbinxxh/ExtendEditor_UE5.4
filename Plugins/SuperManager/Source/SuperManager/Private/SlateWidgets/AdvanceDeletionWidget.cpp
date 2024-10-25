// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "DebugHeader.h"

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
			.VAlign(VAlign_Fill)//填充整个高度
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
	//检查数据资产是否有效，如何无效返回默认值
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	TSharedRef< STableRow< TSharedPtr<FAssetData> >> ListViewRowWidget =
		SNew(STableRow< TSharedPtr<FAssetData> >, OwnerTable).Padding(FMargin(5.f)) //行之间的间隔
		[
			SNew(SHorizontalBox)

			// First slot for check box
			+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)//左对齐
				.VAlign(VAlign_Center)//中心对齐
				.FillWidth(.05f) // 这将决定，这个控件的这个插槽会占用多少空间
			[
				ConstructCheckBox(AssetDataToDisplay)
			]


			// Second slot for displaying asset class name

			// Third slot for displaying asset name
			+SHorizontalBox::Slot()
			[
				SNew(STextBlock)
					.Text(FText::FromString(DisplayAssetName))
			]

			// Fourth slot for a button

		];

	return ListViewRowWidget;
}

//用于构造 SCheckBox 控件
TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)//复选框的类型
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)//当复选框改变状态时调用
	.Visibility(EVisibility::Visible);

	return ConstructedCheckBox;
}

//当选择复选框改变时的回调函数
void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	//通过枚举值来判断复选框选择的状态
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is unchecked"), FColor::Red);
		break;
	case ECheckBoxState::Checked:
		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is Checked"), FColor::Green);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}
