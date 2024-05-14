// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combatable.h"
#include "EnemyBase.generated.h"

UCLASS()
class LIGHTSOULS_API AEnemyBase : public ACombatable
{
	GENERATED_BODY()

public:
	AEnemyBase();

	float GetLightAttackRange() const;

protected:
	virtual void Damage(const float Damage, const FVector& HitterLocation) override;
	virtual void Die() override;

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget>HealthBarWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	class UWidgetComponent* HealthBarInWorld;

	UPROPERTY(EditDefaultsOnly)
	float LightAttackRange = 100.f;
};
