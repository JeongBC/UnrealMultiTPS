# UnrealMultiTPS
  This Project is Unreal Multi Tps C++


------------  

## Vedio Link
*Warning Sound
https://youtu.be/JVfdwmm4Mkw

------------  
## Important code(script)
__Fire Function(for Multi)__

```c++
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
		FVector EyeLocation;
		FRotator EyeRotation;
		myOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);

		FCollisionQueryParams QuereyParams;
		QuereyParams.AddIgnoredActor(myOwner);
		QuereyParams.AddIgnoredActor(this);
		QuereyParams.bTraceComplex = true;
		QuereyParams.bReturnPhysicalMaterial = true;

		//파티클 "target" 파라미터
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType=SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QuereyParams))
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
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
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
```



__OnHealthChanged Function Override(in UE4)__
```c++
void APlayerCharacter::OnHealthChanged(UHealthComponent* HelathComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		//죽었을떄
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);
	}
}
```

__Heal & Check Friendly Fire (OnHealthChanged)__
```c++
//회복 함수
void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health<=0.0f)
	{
		return;
	}
	Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
	UE_LOG(LogTemp, Log, TEXT("체력 회복: %s"), *FString::SanitizeFloat(HealAmount));

	//체력변경 델리게이트 호출 damage 인자값에 마이너스를 넣어 회복
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

//아군 오인사격
bool UHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	UHealthComponent* HealthCompA = Cast<UHealthComponent>(ActorA->GetComponentByClass(UHealthComponent::StaticClass()));
	UHealthComponent* HealthCompB = Cast<UHealthComponent>(ActorB->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		// Assume friendly
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
	//DOREPLIFETIME(ASCharacter, bDied);
}

```

__Change Game State__
```c++
void AJGameModeBase::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		UHealthComponent* HealthComp = Cast<UHealthComponent>(TestPawn->GetComponentByClass(UHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}
}
```
  ------------
## How to Play

W A S D :: Move
Right Mouse :: Zoom
Left Mouse :: Shoot
Space :: Jump
