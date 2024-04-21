// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "PlayerControllerBase.generated.h"
/**
 *
 */
UCLASS()
class LIGHTSOULS_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:

protected:
	virtual void BeginPlay() override;
private:

public:

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget>HudWidgetClass;

private:
	UUserWidget* HudWidget;
};
