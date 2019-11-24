// Fill out your copyright notice in the Description page of Project Settings.

#include "JTrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "PlayerCharacter.h"
#include "HealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AJTrackerBot::AJTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AJTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	ExplosionDamage = 60;
	ExposionRadius = 200;

	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void AJTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();

		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &AJTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}


}

void AJTrackerBot::HandleTakeDamage(UHealthComponent* OwningHelathComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//체력이 0 이면 폭발

	//힘 발생

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}


	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

//다음 목적지 찾기
FVector AJTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || UHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}

		UHealthComponent* TestPawnHealthComp = Cast<UHealthComponent>(TestPawn->GetComponentByClass(UHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &AJTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in the path
			return NavPath->PathPoints[1];
		}
	}

	// Failed to find path
	return GetActorLocation();
}

void AJTrackerBot::SelfDestruct()
{

	if (bExploded)
	{
		return;
	}
	bExploded = true;

	//파티클 생성
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	//사운드 실행
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	//매쉬 비활성화
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);

		//데미지 증가
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		//데미지적용
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExposionRadius, nullptr, IgnoreActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExposionRadius, 12, FColor::Red, false, 2.0F, 0, 1.0F);

		//SelfLifeSpan(2.0f);
	}
	
}

void AJTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void AJTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "target 도달!");
		}
		else
		{

			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation()*ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}
		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 4.0f, 1.0f);
	}
}



void AJTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{

	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded)
	{
		APlayerCharacter* PlayerPawn = Cast<APlayerCharacter>(OtherActor);

		if (PlayerPawn)
		{
			if (Role == ROLE_Authority)
			{
				//스스로 파괴하는 함수 타이머 호출
				GetWorldTimerManager().SetTimer(TimeHandle_SelfDamage, this, &AJTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);

			}	
			bStartedSelfDestruction = true;

			//사운드 실행
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound,RootComponent);
		}	
	}

}

void AJTrackerBot::OnCheckNearbyBots()
{
	// distance to check for nearby bots
	const float Radius = 600;

	// Create temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	// Only find Pawns (eg. players and AI bots)
	FCollisionObjectQueryParams QueryParams;
	// Our tracker bot's mesh component is set to Physics Body in Blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);


	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	

	int32 NrOfBots = 0;
	// loop over the results using a "range based for loop"
	for (FOverlapResult Result : Overlaps)
	{
		// Check if we overlapped with another tracker bot (ignoring players and other bot types)
		AJTrackerBot* Bot = Cast<AJTrackerBot>(Result.GetActor());
		// Ignore this trackerbot instance
		if (Bot && Bot != this)
		{
			NrOfBots++;
		}
	}

	const int32 MaxPowerLevel = 4;

	// Clamp between min=0 and max=4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	// Update the material color
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		// Convert to a float between 0 and 1 just like an 'Alpha' value of a texture. Now the material can be set up without having to know the max power level 
		// which can be tweaked many times by gameplay decisions (would mean we need to keep 2 places up to date)
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}


		// Draw on the bot location
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	
}

void AJTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

