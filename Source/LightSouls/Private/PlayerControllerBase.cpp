// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerControllerBase.h"
#include "Blueprint/UserWidget.h"

void APlayerControllerBase::BeginPlay()
{
	HudWidget = CreateWidget<UUserWidget>(this, HudWidgetClass);
	HudWidget->AddToPlayerScreen();
	HudWidget->SetVisibility(ESlateVisibility::Visible);
}