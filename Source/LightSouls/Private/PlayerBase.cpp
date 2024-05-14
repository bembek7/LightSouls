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
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

// Sets default values
APlayerBase::APlayerBase()
{
	AIPerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AI Perception Stimuli Source"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->bDoCollisionTest = true;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = true;

	RollTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("Roll Timeline"));
	RollInterp.BindUFunction(this, FName("RollUpdate"));
	RollFinishedEvent.BindUFunction(this, FName("RollFinished"));

	CurrentStamina = MaxStamina;
}

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
}

bool APlayerBase::IsLockedOnTarget() const
{
	return IsValid(LockedTarget);
}

float APlayerBase::GetMoveInputX() const
{
	return MoveVector.X;
}

float APlayerBase::GetMoveInputY() const
{
	return MoveVector.Y;
}

void APlayerBase::StartBlocking()
{
	if (HasEnoughStamina(StaminaRequiredToBlock))
	{
		Super::StartBlocking();
	}
}

void APlayerBase::HitBlocked(const float OriginalDamage)
{
	PayStamina(OriginalDamage);

	Super::HitBlocked(OriginalDamage);
}

void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (LockedTarget)
	{
		RotatePlayerToTarget();
	}
	if (bIsBlocking && !HasEnoughStamina(StaminaRequiredToBlock))
	{
		EndBlocking();
	}
}

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
		if (IALightAttack)
		{
			PlayerEnhancedInputComponent->BindAction(IALightAttack, ETriggerEvent::Started, this, &APlayerBase::LightAttackInputResponse);
		}
		if (IAHeavyAttack)
		{
			PlayerEnhancedInputComponent->BindAction(IAHeavyAttack, ETriggerEvent::Started, this, &APlayerBase::HeavyAttackInputResponse);
		}
		if (IALockCamera)
		{
			PlayerEnhancedInputComponent->BindAction(IALockCamera, ETriggerEvent::Started, this, &APlayerBase::LockCameraInputResponse);
		}
		if (IABlock)
		{
			PlayerEnhancedInputComponent->BindAction(IABlock, ETriggerEvent::Started, this, &APlayerBase::StartBlocking);
			PlayerEnhancedInputComponent->BindAction(IABlock, ETriggerEvent::Completed, this, &APlayerBase::EndBlocking);
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

void APlayerBase::EnemyDied(AActor* const DeadEnemy)
{
	if (LockedTarget && DeadEnemy && LockedTarget == DeadEnemy)
	{
		if (AActor* const ClosestEnemy = GetClosestEnemy())
		{
			LockedTarget = ClosestEnemy;
		}
		else
		{
			UnlockCamera();
		}
	}
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
	if (!LockedTarget)
	{
		const FVector2D LookVector = IAValue.Get<FVector2D>();
		AddControllerYawInput(LookVector.X * 1);
		AddControllerPitchInput(LookVector.Y * 1 * -1);
	}
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

		const float AnimLength = RollAnimSequence->GetPlayLength() / RollAnimPlayRate;
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(RollAnimSequence, FName("AllSlot"), 0.25f, 0.25f, RollAnimPlayRate);
		GetCharacterMovement()->StopMovementImmediately();

		RollDirection = Camera->GetRightVector() * MoveVector.X + FVector::CrossProduct(Camera->GetRightVector(), FVector::ZAxisVector) * MoveVector.Y;

		if (LockedTarget && SpringArm)
		{
			SpringArm->bInheritRoll = false;
			SpringArm->bInheritYaw = false;
			SetActorRotation(RollDirection.Rotation());
		}

		RollTimeline->SetTimelineLength(AnimLength - 0.1f);
		RollTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
		RollTimeline->PlayFromStart();
	}
}

void APlayerBase::LightAttackInputResponse()
{
	if (!IsInputBlocked() && HasEnoughStamina(LightAttackStaminaCost))
	{
		PayStamina(LightAttackStaminaCost);

		StartLightAttack();
	}
}

void APlayerBase::HeavyAttackInputResponse()
{
	if (!IsInputBlocked() && HasEnoughStamina(HeavyAttackStaminaCost))
	{
		PayStamina(HeavyAttackStaminaCost);

		StartHeavyAttack();
	}
}

void APlayerBase::RollUpdate(const float RollForceMultiplier) const
{
	GetCharacterMovement()->AddInputVector(RollDirection * RollForceMultiplier, true);
}

void APlayerBase::RollFinished()
{
	GetCharacterMovement()->StopMovementImmediately();
	if (SpringArm)
	{
		SpringArm->bInheritRoll = true;
		SpringArm->bInheritYaw = true;
	}

	bInRoll = false;
}

void APlayerBase::LockCameraInputResponse()
{
	if (LockedTarget)
	{
		UnlockCamera();
	}
	else
	{
		LockCamera();
	}
}

void APlayerBase::UnlockCamera()
{
	if (SpringArm && Camera)
	{
		LockedTarget = nullptr;
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->bInheritPitch = true;
		Camera->bUsePawnControlRotation = true;
	}
}

void APlayerBase::LockCamera()
{
	AActor* const FoundTarget = EstablishTargetToLockOn();
	if (FoundTarget && Camera && SpringArm)
	{
		LockedTarget = FoundTarget;

		SpringArm->bUsePawnControlRotation = false;
		SpringArm->bInheritPitch = false;
		Camera->bUsePawnControlRotation = false;
		Camera->AddRelativeRotation(FRotator(10, 0, 0));
	}
}

void APlayerBase::RotatePlayerToTarget()
{
	if (LockedTarget && SpringArm)
	{
		FRotator RotationToTarget = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LockedTarget->GetActorLocation());

		if (bInRoll)
		{
			RotationToTarget.Pitch -= 40;
			SpringArm->SetRelativeRotation(RotationToTarget);
		}
		else
		{
			SpringArm->SetRelativeRotation(FRotator(RotationToTarget.Pitch - 40, 0, 0));
			RotationToTarget.Pitch = 0;
			SetActorRotation(RotationToTarget);
		}
	}
}

AActor* APlayerBase::EstablishTargetToLockOn()
{
	AActor* FoundTarget = FindEnemyLookedAt();
	if (FoundTarget)
	{
		return FoundTarget;
	}
	else
	{
		return GetClosestEnemy();
	}
}

AActor* APlayerBase::FindEnemyLookedAt()
{
	TArray<FHitResult> HitResults;
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	const FVector CameraForward = Camera->GetForwardVector();
	const FVector CameraLocation = Camera->GetComponentLocation();
	const FVector BoxShape = FVector(1, 300, 300);
	UKismetSystemLibrary::BoxTraceMulti(GetWorld(), CameraLocation,
		CameraLocation + 3000 * CameraForward, BoxShape, Camera->GetComponentRotation(),
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Camera), false, IgnoredActors,
		EDrawDebugTrace::None, HitResults, true);
	AActor* ClosestToCameraDirectionEnemy = nullptr;
	double BestDotWithCameraForward = 0.f;
	for (const auto& HitResult : HitResults)
	{
		if (APawn* FoundEnemy = Cast<APawn>(HitResult.GetActor()))
		{
			FVector EnemyDirection = FoundEnemy->GetActorLocation() - CameraLocation;
			EnemyDirection.Normalize();
			const double DotToCamera = FVector::DotProduct(CameraForward, EnemyDirection);
			if (DotToCamera > BestDotWithCameraForward)
			{
				BestDotWithCameraForward = DotToCamera;
				ClosestToCameraDirectionEnemy = FoundEnemy;
			}
		}
	}
	return ClosestToCameraDirectionEnemy;
}

AActor* APlayerBase::GetClosestEnemy() const
{
	if (EnemyClass)
	{
		const float SearchedDistance = 1500.f;
		float CurrentShortestDistance = SearchedDistance;
		AActor* ClosestEnemy = nullptr;
		TArray<AActor*> FoundEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemyClass, FoundEnemies);

		for (const auto Enemy : FoundEnemies)
		{
			const float DistanceToEnemy = FVector::Distance(GetActorLocation(), Enemy->GetActorLocation());
			if (DistanceToEnemy < SearchedDistance && DistanceToEnemy < CurrentShortestDistance)
			{
				if (AEnemyBase* const CastedEnemy = Cast<AEnemyBase>(Enemy))
				{
					if (!CastedEnemy->IsDead())
					{
						CurrentShortestDistance = DistanceToEnemy;
						ClosestEnemy = Enemy;
					}
				}
			}
		}
		return ClosestEnemy;
	}
	return nullptr;
}

bool APlayerBase::IsInputBlocked() const
{
	return bInAttack || bInRoll;
}