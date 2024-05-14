// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthBar.h"
#include "Combatable.h"

void UHealthBar::SetOwner(APawn* const NewOwner)
{
	Owner = NewOwner;
}

float UHealthBar::CalculateHealthPercent() const
{
	if (ACombatable* OwningPawn = Cast<ACombatable>(Owner))
	{
		return OwningPawn->GetCurrentHealth() / OwningPawn->GetMaxHealth();
	}
	return 1.f;
}