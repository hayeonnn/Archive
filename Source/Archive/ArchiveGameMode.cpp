// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchiveGameMode.h"
#include "ArchiveCharacter.h"
#include "UObject/ConstructorHelpers.h"

AArchiveGameMode::AArchiveGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
