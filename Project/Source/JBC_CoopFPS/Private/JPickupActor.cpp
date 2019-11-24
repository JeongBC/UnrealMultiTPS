// Fill out your copyright notice in the Description page of Project Settings.

#include "JPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "JPowerUpActor.h"
#include "TimerManager.h"

// Sets default values
AJPickupActor::AJPickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);

	CooldownDuration = 10.0f;

	SetReplicates(true);;
}

// Called when the game starts or when spawned
void AJPickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		Respawn();
	}
}

void AJPickupActor::Respawn()
{
	if (PowerUpClass == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your Blueprint"), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerUpInstance = GetWorld()->SpawnActor<AJPowerUpActor>(PowerUpClass, GetTransform(), SpawnParams);
}

void AJPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Role == ROLE_Authority && PowerUpInstance)
	{
		PowerUpInstance->ActivatePowerup(OtherActor);
		PowerUpInstance = nullptr;

		// 파워업 엑터 재생성
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &AJPickupActor::Respawn, CooldownDuration);
	}
}

