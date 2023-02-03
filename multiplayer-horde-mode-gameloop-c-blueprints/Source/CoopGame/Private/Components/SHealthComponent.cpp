// Creative Commons - Reconocimiento (by)

#include "../CoopGame/Public/Components/SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "../CoopGame/Public/SCharacter.h"
#include "SGameMode.h"

static int32 DebugHealthComponentDrawing = 1;
FAutoConsoleVariableRef CVARDebugHealthComponentDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugHealthComponentDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat
);

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{

	DefaultHealth = 100.0f;
	SetIsReplicated(true);
	bIsDead = false;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook if we are server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	
	bool bCharacterShield = false;
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{

		ASCharacter* MyOwnerSCharacter = Cast<ASCharacter>(MyOwner);
		if (MyOwnerSCharacter)
		{
			bCharacterShield = MyOwnerSCharacter->IsShieldActive();
		}

	}



	if (Damage <= 0.0f || bCharacterShield || bIsDead)
	{
		return;
	}

	// Update Health Clamped
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	//UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
	if (DebugHealthComponentDrawing > 0) UE_LOG(LogTemp, Log, TEXT("Object State: Name= %s - Health=%s"), *GetOwner()->GetName(), *FString::SanitizeFloat(Health));

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);

}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}

