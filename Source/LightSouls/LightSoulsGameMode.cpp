// Copyright Epic Games, Inc. All Rights Reserved.

#include "LightSoulsGameMode.h"
#include "LightSoulsCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALightSoulsGameMode::ALightSoulsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
