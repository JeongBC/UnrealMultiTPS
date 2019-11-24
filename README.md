# UnrealMultiTPS
  This Project is Unreal Multi Tps C++


------------  

## Vedio Link

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
  ------------
## How to Play

W A S D :: Move
Right Mouse :: Zoom
Left Mouse :: Shoot
Space :: Jump
