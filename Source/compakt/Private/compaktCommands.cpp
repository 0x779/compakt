// Copyright Epic Games, Inc. All Rights Reserved.

#include "compaktCommands.h"

#define LOCTEXT_NAMESPACE "FcompaktModule"

void FcompaktCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "compakt", "Pack folder to .upack", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
