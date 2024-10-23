// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"


void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true; // 支持键盘焦点

	ChildSlot
	[
		SNew(STextBlock).Text(FText::FromString(InArgs._TestString))
	];
}
