// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "JGameModeBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.0f;
	bIsDead = false;

	TeamNum = 255;
	SetIsReplicated(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		}
	}
	
	Health = MaxHealth;

}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health,Damage , nullptr, NULL, nullptr);
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead)
	{
		return;
	}

	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	// Update health clamped
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		AJGameModeBase* GM = Cast<AJGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

//ȸ�� �Լ�
void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health<=0.0f)
	{
		return;
	}
	Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
	UE_LOG(LogTemp, Log, TEXT("ü�� ȸ��: %s"), *FString::SanitizeFloat(HealAmount));

	//ü�º��� ��������Ʈ ȣ�� damage ���ڰ��� ���̳ʽ��� �־� ȸ��
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

//�Ʊ� ���λ��
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

