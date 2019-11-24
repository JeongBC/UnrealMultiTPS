// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class AJPowerUpActor;

UCLASS()
class JBC_COOPFPS_API AJPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		TSubclassOf<AJPowerUpActor> PowerUpClass;

	AJPowerUpActor* PowerUpInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		float CooldownDuration;

	//리스폰용 타이머 핸들러
	FTimerHandle TimerHandle_RespawnTimer;

	
	void Respawn();


public:	

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
};
