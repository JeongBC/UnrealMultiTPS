// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JPowerUpActor.generated.h"


UCLASS()
class JBC_COOPFPS_API AJPowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJPowerUpActor();

protected:
	/* Time between powerup ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		float PowerupInterval;

	/* Total times we apply the powerup effect */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		int32 TotalNrOfTicks;

	FTimerHandle TimerHandle_PowerupTick;

	// Total number of ticks applied
	int32 TicksProcessed;

	UFUNCTION()
		void OnTickPowerup();

	// Keeps state of the power-up
	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
		bool bIsPowerupActive;

	UFUNCTION()
		void OnRep_PowerupActive();

	/* 블루프린트 이벤트 호출용 함수 - 플레이어의 상태 변경*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupStateChanged(bool bNewIsActive);

public:	
	void ActivatePowerup(AActor* ActiveFor);

	/* 블루프린트 이벤트 호출용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnActivated(AActor* ActiveFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnExpired();
	
};
