// Fill out your copyright notice in the Description page of Project Settings.

#include "ExplosiveObject.h"
#include "HealthComponent.h"
#include "Kismet//GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AExplosiveObject::AExplosiveObject()
{
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveObject::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);

	RootComponent = MeshComp;

	RadialComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialComp"));
	RadialComp->SetupAttachment(MeshComp);
	RadialComp->Radius = 250;
	RadialComp->bImpulseVelChange = true;
	RadialComp->bAutoActivate = false;
	RadialComp->bIgnoreOwningActor = true;

	ExplosionImpulse = 400;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AExplosiveObject::OnHealthChanged(UHealthComponent* OwningHelathComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}

	if (Health <= 0.0f)
	{
		bExploded = true;
		OnRep_Exploded();

		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		

		RadialComp->FireImpulse();
	}
}

void AExplosiveObject::OnRep_Exploded()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	MeshComp->SetMaterial(0, ExplodedMaterial);
}

void AExplosiveObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplosiveObject, bExploded);
	//DOREPLIFETIME(ASCharacter, bDied);
}