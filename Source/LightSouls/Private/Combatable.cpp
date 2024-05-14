// Fill out your copyright notice in the Description page of Project Settings.

#include "Combatable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles\ParticleSystemComponent.h"
#include "Components/CapsuleComponent.h"

ACombatable::ACombatable()
{
	PrimaryActorTick.bCanEverTick = true;

	SwordCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Sword"));
	SwordCollider->SetupAttachment(GetMesh(), FName("Sword_joint"));
	SwordCollider->SetGenerateOverlapEvents(true);
	SwordCollider->SetCollisionProfileName(FName("OverlapAll"));
	SwordCollider->SetRelativeLocation(FVector(46, -20, 5));
	SwordCollider->SetRelativeRotation(FRotator(4, -14, 0.2));
	SwordCollider->SetBoxExtent(FVector(35, 3, 1));

	CurrentHealth = MaxHealth;
}

// Called when the game starts or when spawned
void ACombatable::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate OnSwordHitDelegate;
	OnSwordHitDelegate.BindUFunction(this, FName("OnSwordHit"));
	if (SwordCollider)
	{
		SwordCollider->OnComponentBeginOverlap.AddUnique(OnSwordHitDelegate);
	}
}

float ACombatable::GetCurrentHealth() const
{
	return CurrentHealth;
}

float ACombatable::GetMaxHealth() const
{
	return MaxHealth;
}

bool ACombatable::IsBlocking() const
{
	return bIsBlocking;
}

void ACombatable::AttackFinished()
{
	bInAttack = false;
	bInLightAttack = false;
	bInHeavyAttack = false;
}

void ACombatable::StartBlocking()
{
	bIsBlocking = true;
	if (BlockingStartAnimSequence)
	{
		const float AnimLength = BlockingStartAnimSequence->GetPlayLength();
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(BlockingStartAnimSequence, FName("UpperSlot"));
	}
}

void ACombatable::EndBlocking()
{
	bIsBlocking = false;
	if (BlockingEndAnimSequence)
	{
		const float AnimLength = BlockingEndAnimSequence->GetPlayLength();
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(BlockingEndAnimSequence, FName("UpperSlot"));
	}
}

void ACombatable::OnSwordHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (bInAttack)
	{
		if (ACombatable* const EnemyHit = Cast<ACombatable>(OtherActor))
		{
			if (!EnemiesHit.Contains(EnemyHit) && EnemyHit != this)
			{
				float AttackDamage = 0.f;
				if (bInLightAttack)
				{
					AttackDamage = LightAttackDamage;
				}
				else
				{
					if (bInHeavyAttack)
					{
						AttackDamage = HeavyAttackDamage;
					}
				}
				EnemyHit->Damage(AttackDamage, GetActorLocation());
				EnemiesHit.Add(EnemyHit);
			}
		}
	}
}

float ACombatable::StartLightAttack()
{
	bInLightAttack = true;

	return StartAttack(LightAttackAnimSequence);
}

float ACombatable::StartHeavyAttack()
{
	bInHeavyAttack = true;

	return StartAttack(HeavyAttackAnimSequence);
}

void ACombatable::HitBlocked(float OriginalDamage)
{
	OriginalDamage -= BlockValue;
	OriginalDamage = FMath::Max(0, OriginalDamage);
	CurrentHealth -= OriginalDamage;
	if (BlockedImpactAnimSequence)
	{
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(BlockedImpactAnimSequence, FName("AllSlot"));
	}

	if (CurrentHealth <= 0)
	{
		Die();
	}
}

float ACombatable::StartAttack(UAnimSequence* const AttackAnimSequence)
{
	bInAttack = true;
	EnemiesHit.Empty();

	GetCharacterMovement()->StopMovementImmediately();

	float AnimLength = 0.f;
	if (AttackAnimSequence)
	{
		AnimLength = AttackAnimSequence->GetPlayLength();
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(AttackAnimSequence, FName("AllSlot"));
		FTimerDelegate AttackDelegate;
		AttackDelegate.BindUFunction(this, FName("AttackFinished"));
		FTimerHandle AttackHandle;
		GetWorldTimerManager().SetTimer(AttackHandle, AttackDelegate, AnimLength, false);
	}

	return AnimLength;
}

bool ACombatable::IsDead() const
{
	return bIsDead;
}

void ACombatable::Damage(const float Damage, const FVector& HitterLocation)
{
	const FVector HitterDirection = HitterLocation - GetActorLocation();
	if (bIsBlocking && FVector::DotProduct(HitterDirection, GetActorForwardVector()) > 0)
	{
		HitBlocked(Damage);
	}
	else
	{
		CurrentHealth -= Damage;
		if (HitImpactAnimSequence)
		{
			GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(HitImpactAnimSequence, FName("AllSlot"));
		}
		SpawnBlood(HitterLocation);
	}

	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void ACombatable::SpawnBlood(const FVector& HitterLocation)
{
	AActor* BloodVFX = GetWorld()->SpawnActor<AActor>(OnHitParticleEffectClass.Get(), FTransform(GetActorLocation()));
	if (BloodVFX)
	{
		USceneComponent* VFXComponent = Cast<USceneComponent>(BloodVFX->GetComponentByClass(UParticleSystemComponent::StaticClass()));
		if (VFXComponent)
		{
			FVector HitDirection = GetActorLocation() - HitterLocation;
			HitDirection.Normalize();
			FRotator BloodRotation = HitDirection.Rotation();
			BloodRotation.Pitch += 90; // VFX is facing up by default so we have to tweak it to be facing forward
			VFXComponent->SetWorldRotation(BloodRotation);
			VFXComponent->Activate();

			FTimerHandle BloodDestructionHandle;
			GetWorldTimerManager().SetTimer(BloodDestructionHandle, [BloodVFX]() {BloodVFX->Destroy(); }, 1.f, false);
		}
	}
}

void ACombatable::Die()
{
	bIsDead = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetGenerateOverlapEvents(false);
	if (DeathAnimSequence)
	{
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(DeathAnimSequence, FName("AllSlot"));
	}
	FTimerHandle DeathHandle;
	GetWorldTimerManager().SetTimer(DeathHandle, [this]() {Destroy(); }, 5.f, false);
}