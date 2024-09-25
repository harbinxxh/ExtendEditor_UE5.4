#pragma once

/*
	说明：到目前为止，代码可以很好的编译
	但稍后，当我们把 DebugHeader.h 文件包含在其他的文件中时，当我们编译会得到一些链接错误。
*/

void Print(const FString& Message, const FColor& Color)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
	}
}

void PrintLog(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}