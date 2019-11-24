// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "Components/AudioComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "JBC_CoopFPS.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"


int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapon"), DebugWeaponDrawing, 
	TEXT("Draw Debug Line for Weapon"),ECVF_Cheat);

// Sets default values
AWeapon::AWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));	
	RootComponent = MeshComp;

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);

	

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;

	RateOfFire = 600;

	MaxAmmo = 30;
	CurrentAmmo = MaxAmmo;

	//클라도 호출해주기
	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0F;

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AudioComp->SetSound(SoundCue);
	TimeBetweenShots = 60 / RateOfFire;
	
}

void AWeapon::Fire()
{
	// 폰의 눈에서 크로스 헤어로 레이 발사

	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* myOwner = GetOwner();

	if (myOwner)
	{
		FVector eyeLocation;
		FRotator EyeRotation;
		myOwner->GetActorEyesViewPoint(eyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = eyeLocation + (EyeRotation.Vector() * 10000);

		FCollisionQueryParams QuereyParams;
		QuereyParams.AddIgnoredActor(myOwner);
		QuereyParams.AddIgnoredActor(this);
		QuereyParams.bTraceComplex = true;
		QuereyParams.bReturnPhysicalMaterial = true;

		//파티클 "target" 파라미터
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType=SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, eyeLocation, TraceEnd, COLLISION_WEAPON, QuereyParams))
		{
			//hit한 대상에 데미지 판정
			AActor* HitActor = Hit.GetActor();
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			
			
			//헤드샷일경우::추가데미지
			float ActaulDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActaulDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActaulDamage, ShotDirection, Hit, myOwner->GetInstigatorController(), this, DamageType);
		
			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

			
		}
		//라인디버그
		if (DebugWeaponDrawing>0)
		{
			DrawDebugLine(GetWorld(), eyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}
		
		PlayFireEffects(TracerEndPoint);

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
			HitScanTrace.Seed = (HitScanTrace.Seed + 1) % 3;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}

	
}

void AWeapon::OnRep_HitScanTrace()
{
	// cosmetic fx
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void AWeapon::ServerFire_Implementation()
{
	Fire();
}

bool AWeapon::ServerFire_Validate()
{
	return true;
}

void AWeapon::StartFire()
{
	float firestDelay = FMath::Max( LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);
	GetWorldTimerManager().SetTimer(TimeHandle_TimeBetweenShots, this,&AWeapon::Fire,TimeBetweenShots,true ,firestDelay);
}

void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimeHandle_TimeBetweenShots);
}

void AWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	AudioComp->Play(0.0F);

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}


	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}

		
	}

	

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
	
}

void AWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{

	UParticleSystem* SelectedEffect = nullptr;
	//맞은 부위 확인
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint,ShotDirection.Rotation());
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, HitScanTrace, COND_SkipOwner);
}
