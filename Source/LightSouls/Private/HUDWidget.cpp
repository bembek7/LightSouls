// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "Components/ProgressBar.h"
#include "PlayerBase.h"
#include "HealthBar.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

float UHUDWidget::CalculateStaminaPercent() const
{
	if (APlayerBase* PlayerPawn = Cast<APlayerBase>(GetOwningPlayerPawn()))
	{
		return PlayerPawn->GetCurrentStamina() / PlayerPawn->GetMaxStamina();
	}
	return 1.f;
}

float UHUDWidget::CalculateHealthPercent() const
{
	if (APlayerBase* PlayerPawn = Cast<APlayerBase>(GetOwningPlayerPawn()))
	{
		return PlayerPawn->GetCurrentHealth() / PlayerPawn->GetMaxHealth();
	}
	return 1.f;
}
