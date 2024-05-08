// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 *
 */
UCLASS()
class LIGHTSOULS_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOwner(APawn* const NewOwner);

protected:
	UFUNCTION(BlueprintPure)
	float CalculateHealthPercent() const;

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar;

private:
	APawn* Owner;
};
