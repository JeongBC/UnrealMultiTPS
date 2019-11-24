// Fill out your copyright notice in the Description page of Project Settings.

#include "JGameState.h"
#include "Net/UnrealNetwork.h"



void AJGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}


void AJGameState::SetWaveState(EWaveState NewState)
{
	if (Role == ROLE_Authority)
	{
		EWaveState OldState = WaveState;

		WaveState = NewState;
		// Call on server
		OnRep_WaveState(OldState);
	}
}

void AJGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJGameState, WaveState);
}