// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "Components/ProgressBar.h"
#include "PlayerBase.h"

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