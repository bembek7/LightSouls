// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

/**
 *
 */
UCLASS()
class LIGHTSOULS_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION(BlueprintPure)
	float CalculateStaminaPercent() const;
private:

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* StaminaBar;
};
