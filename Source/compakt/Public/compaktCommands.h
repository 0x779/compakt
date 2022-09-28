// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "compaktStyle.h"

class FcompaktCommands : public TCommands<FcompaktCommands>
{
public:

	FcompaktCommands()
		: TCommands<FcompaktCommands>(TEXT("compakt"), NSLOCTEXT("Contexts", "compakt", "compakt Plugin"), NAME_None, FcompaktStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
