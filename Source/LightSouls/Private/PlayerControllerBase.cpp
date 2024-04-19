// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* const LocalPlayer = Cast<ULocalPlayer>(GetLocalPlayer()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* const InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!InputMappingContext.IsNull())
			{
				InputSystem->AddMappingContext(InputMappingContext.LoadSynchronous(), 2);
			}
		}
	}
}

void APlayerControllerBase::SetupInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* const PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IAWalk)
		{
			PlayerEnhancedInputComponent->BindAction(IAWalk, ETriggerEvent::Triggered, this, &APlayerControllerBase::Walk);
		}
		if (IALook)
		{
			PlayerEnhancedInputComponent->BindAction(IALook, ETriggerEvent::Triggered, this, &APlayerControllerBase::Look);
		}
	}
}


void APlayerControllerBase::Walk(const FInputActionValue& IAValue)
{
	const FVector2D MoveVector = IAValue.Get<FVector2D>();
	if (APawn* const PlayerPawn = GetPawn())
	{
		PlayerPawn->AddMovementInput(PlayerPawn->GetActorRightVector(), MoveVector.X);
		PlayerPawn->AddMovementInput(PlayerPawn->GetActorForwardVector(), MoveVector.Y);
	}
}

void APlayerControllerBase::Look(const FInputActionValue& IAValue)
{
	const FVector2D LookVector = IAValue.Get<FVector2D>();
	if (APawn* const PlayerPawn = GetPawn())
	{
		PlayerPawn->AddControllerYawInput(LookVector.X * 1);
		PlayerPawn->AddControllerPitchInput(LookVector.Y * 1 * -1);
	}
}
