// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "HealthBar.h"

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

