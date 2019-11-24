// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "GrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class JBC_COOPFPS_API AGrenadeLauncher : public AWeapon
{
	GENERATED_BODY()
	
protected:
	virtual void Fire() override;
	
	UPROPERTY(EditDefaultsOnly,Category="GrenadeLancher")
	TSubclassOf<AActor> ProjectileClass;
};
