// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetAction.h"
#include "DebugHeader.h"

void UQuickAssetAction::TestFunc()
{
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, TEXT("Working"));
	//}

	Print(TEXT("Working"), FColor::Cyan);
	PrintLog(TEXT("Working"));
}
