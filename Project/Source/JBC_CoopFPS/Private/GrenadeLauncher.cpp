// Fill out your copyright notice in the Description page of Project Settings.

#include "GrenadeLauncher.h"




void AGrenadeLauncher::Fire()
{
	AActor* myOwner = GetOwner();

	if (myOwner&&ProjectileClass)
	{
		FVector eyeLocation;
		FRotator EyeRotation;
		myOwner->GetActorEyesViewPoint(eyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}

}
