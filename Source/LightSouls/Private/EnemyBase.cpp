// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "HealthBar.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerBase.h"
#include "EnemyControllerBase.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
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

float AEnemyBase::GetLightAttackRange() const
{
	return LightAttackRange;
}

void AEnemyBase::Damage(const float Damage, const FVector& HitterLocation)
{
	Super::Damage(Damage, HitterLocation);
	if (!bIsDead)
	{
		if (AEnemyControllerBase* const EnemyController = Cast<AEnemyControllerBase>(GetController()))
		{
			EnemyController->StopBehaviour();
			FTimerHandle RestartBehaviourHandle;
			GetWorldTimerManager().SetTimer(RestartBehaviourHandle, [EnemyController]() {EnemyController->RestartBehaviour(); }, HitImpactAnimSequence->GetPlayLength(), false);
		}
	}
}

void AEnemyBase::Die()
{
	Super::Die();
	if (AEnemyControllerBase* const EnemyController = Cast<AEnemyControllerBase>(GetController()))
	{
		EnemyController->StopBehaviour();
	}
	if (APlayerBase* const PlayerPawn = Cast<APlayerBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		PlayerPawn->EnemyDied(this);
	}
}