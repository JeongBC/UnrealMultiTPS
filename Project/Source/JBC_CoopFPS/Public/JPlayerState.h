// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "JPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class JBC_COOPFPS_API AJPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	
public:

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
		void AddScore(float ScoreDelta);
	
};
