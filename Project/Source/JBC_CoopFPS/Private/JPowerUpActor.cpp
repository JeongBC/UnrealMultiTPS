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

	//�������Ʈ �̺�Ʈ ȣ�� OnPowerupTicked
	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		//Ÿ�̸� ����
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void AJPowerUpActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void AJPowerUpActor::ActivatePowerup(AActor* ActiveFor)
{
	//�������Ʈ �Լ� ȣ��
	OnActivated(ActiveFor);

	bIsPowerupActive = true;

	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		//�Ŀ� �ð� ������ Ÿ�̸� ����(�������Ʈ �Լ� OnTickPowerup ����)
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &AJPowerUpActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		//�÷��̾� �Ŀ� ��
		OnTickPowerup();
	}
}

void AJPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJPowerUpActor, bIsPowerupActive);
}

