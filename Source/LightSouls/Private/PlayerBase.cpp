// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerControllerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
APlayerBase::APlayerBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bDoCollisionTest = true;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = true;

	RollTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("RollTimeline"));
	RollInterp.BindUFunction(this, FName("RollUpdate"));
	RollFinishedEvent.BindUFunction(this, FName("RollFinished"));
}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	if (RollForceCurve)
	{
		RollTimeline->AddInterpFloat(RollForceCurve, RollInterp, FName("Force"), FName("ForceTrack"));
	}
	RollTimeline->SetTimelineFinishedFunc(RollFinishedEvent);
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* const OwningController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* const LocalPlayer = Cast<ULocalPlayer>(OwningController->GetLocalPlayer()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (!InputMappingContext.IsNull())
				{
					InputSystem->AddMappingContext(InputMappingContext.LoadSynchronous(), 1);
				}
			}
		}
	}

	if (UEnhancedInputComponent* const PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IAWalk)
		{
			PlayerEnhancedInputComponent->BindAction(IAWalk, ETriggerEvent::Triggered, this, &APlayerBase::Walk);
		}
		if (IALook)
		{
			PlayerEnhancedInputComponent->BindAction(IALook, ETriggerEvent::Triggered, this, &APlayerBase::Look);
		}
		if (IARoll)
		{
			PlayerEnhancedInputComponent->BindAction(IARoll, ETriggerEvent::Started, this, &APlayerBase::StartRoll);
		}
	}
}

void APlayerBase::Walk(const FInputActionValue& IAValue)
{
	if (!bInRoll)
	{
		MoveVector = IAValue.Get<FVector2D>();
		if (Camera)
		{
			AddMovementInput(Camera->GetRightVector(), MoveVector.X);
			AddMovementInput(FVector::CrossProduct(Camera->GetRightVector(), FVector::ZAxisVector), MoveVector.Y);
		}
	}
}

void APlayerBase::Look(const FInputActionValue& IAValue)
{
	const FVector2D LookVector = IAValue.Get<FVector2D>();
	AddControllerYawInput(LookVector.X * 1);
	AddControllerPitchInput(LookVector.Y * 1 * -1);
}

void APlayerBase::StartRoll()
{
	if (!bInRoll)
	{
		bInRoll = true;
		const float AnimLength = PlayAnimMontage(RollAnimMontage, RollAnimPlayRate) / RollAnimPlayRate;
		GetCharacterMovement()->StopMovementImmediately();

		RollDirection = Camera->GetRightVector() * MoveVector.X + FVector::CrossProduct(Camera->GetRightVector(), FVector::ZAxisVector) * MoveVector.Y;

		RollTimeline->SetTimelineLength(AnimLength - 0.14f);
		RollTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
		RollTimeline->PlayFromStart();
	}
}

void APlayerBase::RollUpdate(const float RollForceMultiplier) const
{
	GetCharacterMovement()->AddInputVector(RollDirection * RollForceMultiplier, true);
}

void APlayerBase::RollFinished()
{
	bInRoll = false;
}