// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveObject.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;

UCLASS()
class JBC_COOPFPS_API AExplosiveObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveObject();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
		UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialComp;

	UFUNCTION()
		void OnHealthChanged(UHealthComponent* OwningHelathComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
		void OnRep_Exploded();

	UPROPERTY(EditDefaultsOnly, Category = "FX")
		float  ExplosionImpulse;
	
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UMaterialInterface* ExplodedMaterial;
};
