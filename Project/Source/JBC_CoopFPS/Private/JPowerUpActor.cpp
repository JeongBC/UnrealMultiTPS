// Fill out your copyright notice in the Description page of Project Settings.

#include "JPowerUpActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AJPowerUpActor::AJPowerUpActor()
{

	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	SetReplicates(true);

}

void AJPowerUpActor::OnTickPowerup()
{
	TicksProcessed++;

	//블루프린트 이벤트 호출 OnPowerupTicked
	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		//타이머 삭제
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void AJPowerUpActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void AJPowerUpActor::ActivatePowerup(AActor* ActiveFor)
{
	//블루프린트 함수 호출
	OnActivated(ActiveFor);

	bIsPowerupActive = true;

	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		//파워 시간 간격후 타이머 실행(블루프린트 함수 OnTickPowerup 실행)
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &AJPowerUpActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		//플레이어 파웡 온
		OnTickPowerup();
	}
}

void AJPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJPowerUpActor, bIsPowerupActive);
}

