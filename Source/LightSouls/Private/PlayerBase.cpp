// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerControllerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnemyBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"

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

	RollTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("Roll Timeline"));
	RollInterp.BindUFunction(this, FName("RollUpdate"));
	RollFinishedEvent.BindUFunction(this, FName("RollFinished"));

	SwordCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Sword"));
	SwordCollider->SetupAttachment(GetMesh(), FName("SwordJoint"));
	SwordCollider->SetGenerateOverlapEvents(true);
	SwordCollider->SetCollisionProfileName(FName("OverlapAll"));
	SwordCollider->SetRelativeLocation(FVector(46, -20, 5));
	SwordCollider->SetRelativeRotation(FRotator(4, -14, 0.2));
	SwordCollider->SetBoxExtent(FVector(35, 3, 1));

	CurrentStamina = MaxStamina;
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

	FTimerDelegate StaminaRegenerationDelegate;
	StaminaRegenerationDelegate.BindUFunction(this, FName("RegenerateStamina"));
	GetWorldTimerManager().SetTimer(StaminaRegenerationHandle, StaminaRegenerationDelegate, 0.1f, true);

	FScriptDelegate OnSwordHitDelegate;
	OnSwordHitDelegate.BindUFunction(this, FName("OnSwordHit"));
	if(SwordCollider)
	{
		SwordCollider->OnComponentBeginOverlap.AddUnique(OnSwordHitDelegate);
	}
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
		if (IARoll)
		{
			PlayerEnhancedInputComponent->BindAction(IALightAttack, ETriggerEvent::Started, this, &APlayerBase::StartLightAttack);
		}
	}
}

float APlayerBase::GetCurrentStamina() const
{
	return CurrentStamina;
}

float APlayerBase::GetMaxStamina() const
{
	return MaxStamina;
}

void APlayerBase::Walk(const FInputActionValue& IAValue)
{
	if (!IsInputBlocked())
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

void APlayerBase::RegenerateStamina()
{
	if (!IsInputBlocked())
	{
		CurrentStamina += StaminaRegenerationPerSecond / 10;
		CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);
	}
}

bool APlayerBase::HasEnoughStamina(const float StaminaCost) const
{
	return CurrentStamina >= StaminaCost;
}

void APlayerBase::PayStamina(const float StaminaCost)
{
	CurrentStamina -= StaminaCost;
	CurrentStamina = FMath::Max(CurrentStamina, 0);
}

void APlayerBase::StartRoll()
{
	if (!IsInputBlocked() && HasEnoughStamina(RollStaminaCost))
	{
		PayStamina(RollStaminaCost);
		bInRoll = true;
		const float AnimLength = PlayAnimMontage(RollAnimMontage, RollAnimPlayRate) / RollAnimPlayRate;
		GetCharacterMovement()->StopMovementImmediately();

		RollDirection = Camera->GetRightVector() * MoveVector.X + FVector::CrossProduct(Camera->GetRightVector(), FVector::ZAxisVector) * MoveVector.Y;

		RollTimeline->SetTimelineLength(AnimLength - 0.1f);
		RollTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
		RollTimeline->PlayFromStart();
	}
}

void APlayerBase::StartLightAttack()
{
	if (!IsInputBlocked() && HasEnoughStamina(LightAttackStaminaCost))
	{
		PayStamina(LightAttackStaminaCost);

		bInAttack = true;
		EnemiesHit.Empty();

		const float AnimLength = PlayAnimMontage(LightAttackAnimMontage);
		GetCharacterMovement()->StopMovementImmediately();

		FTimerDelegate AttackDelegate;
		AttackDelegate.BindUFunction(this, FName("AttackFinished"));
		FTimerHandle AttackHandle;
		GetWorldTimerManager().SetTimer(AttackHandle, AttackDelegate, AnimLength, false);
	}
}

void APlayerBase::RollUpdate(const float RollForceMultiplier) const
{
	GetCharacterMovement()->AddInputVector(RollDirection * RollForceMultiplier, true);
}

void APlayerBase::RollFinished()
{
	GetCharacterMovement()->StopMovementImmediately();
	bInRoll = false;
}

void APlayerBase::AttackFinished()
{
	bInAttack = false;
}

bool APlayerBase::IsInputBlocked() const
{
	return bInAttack || bInRoll;
}

void APlayerBase::OnSwordHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if(bInAttack)
	{
		if (AEnemyBase* const EnemyHit = Cast<AEnemyBase>(OtherActor))
		{
			if(!EnemiesHit.Contains(EnemyHit))
			{
				EnemyHit->Damage(LightAttackDamage, GetActorLocation());
				EnemiesHit.Add(EnemyHit);
			}
		}
	}
}
