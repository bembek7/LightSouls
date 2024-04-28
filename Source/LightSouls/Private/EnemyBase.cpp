// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "HealthBar.h"
#include "Particles\ParticleSystemComponent.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrentHealth = MaxHealth;

	HealthBarInWorld = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarInWorld->SetupAttachment(GetCapsuleComponent());
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarInWorld)
	{
		HealthBarInWorld->SetWidgetClass(HealthBarWidgetClass);
		if (UHealthBar* HealthBarWidget = Cast<UHealthBar>(HealthBarInWorld->GetWidget()))
		{
			HealthBarWidget->SetOwner(this);
		}
	}
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AEnemyBase::GetCurrentHealth() const
{
	return CurrentHealth;
}

float AEnemyBase::GetMaxHealth() const
{
	return MaxHealth;
}

void AEnemyBase::Damage(const float Damage, const FVector& HitterLocation)
{
	CurrentHealth -= Damage;
	if (CurrentHealth <= 0)
	{
		Die();
	}
	else
	{
		if (HitImpactAnimMontage)
		{
			PlayAnimMontage(HitImpactAnimMontage);
		}
	}

	SpawnBlood(HitterLocation);
}

void AEnemyBase::SpawnBlood(const FVector& HitterLocation)
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

void AEnemyBase::Die()
{
	bIsDead = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (DeathAnimMontage)
	{
		PlayAnimMontage(DeathAnimMontage);
	}
	FTimerHandle DeathHandle;
	GetWorldTimerManager().SetTimer(DeathHandle, [this]() {Destroy(); }, 5.f, false);
}