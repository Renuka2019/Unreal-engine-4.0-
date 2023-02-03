// Creative Commons - Reconocimiento (by)

#include "../CoopGame/Public/AI/STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "../CoopGame/Public/Components/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "../CoopGame/Public/SCharacter.h"
#include "Sound/SoundCue.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "CoopGame.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugTrackerBotDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat
);

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this,&ASTrackerBot::HandleTakeDame);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	SphereNearBotComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereNearBotComp"));
	SphereNearBotComp->SetSphereRadius(600);
	SphereNearBotComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ECollisionChannel CollisionChannel = BOT_OVERLAP;
	SphereNearBotComp->SetCollisionObjectType(CollisionChannel);
	SphereNearBotComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereNearBotComp->SetCollisionResponseToChannel(BOT_OVERLAP, ECR_Overlap);
	SphereNearBotComp->SetupAttachment(RootComponent);

	bUseVeloctiyChange = true;
	MovementForce = 400;
	RequiredDistanceToTarget = 200;

	ExplosionDamage = 20;
	ExplosionRadius = 600;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("URadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bIgnoreOwningActor = true;
	RadialForceComp->bAutoActivate = false;

	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

	FTimerHandle TimerHandle_CheckPowerLevel;
	GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::onCheckNearbyBots, 1.0f, true);

	if (GetLocalRole() == ROLE_Authority)
	{
		// Find initial move to
		NextPathPoint = GetNextPathPoint();
	}
}

void ASTrackerBot::HandleTakeDame(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);

	}

	// Explode on hitpoints == 0
	if (Health <= 0.0f)
	{
		SelfDesctruct();
	}

}

FVector ASTrackerBot::GetNextPathPoint()
{
	// Hack, to get player location
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (PlayerPawn)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in the path
			return NavPath->PathPoints[1];
		}

		// Fail to find path
		return GetActorLocation();
	}
	else {
		return FVector::ZeroVector;
	}
	
}

void ASTrackerBot::SelfDesctruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false,true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetLocalRole() == ROLE_Authority)
	{

		// Apply Damage!
		if (DebugTrackerBotDrawing > 0) UE_LOG(LogTemp, Log, TEXT("Execute destruction,Base %s - PowerLevelUp : %s"), *FString::SanitizeFloat(ExplosionDamage), *FString::SanitizeFloat(PowerLevel));
		
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		float FinalDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);
		UGameplayStatics::ApplyRadialDamage(this, FinalDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		
		RadialForceComp->FireImpulse();

		// Delete Actor inmediately
		SetLifeSpan(2.0f);

		if (DebugTrackerBotDrawing > 0)DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(),this, nullptr);
}

void ASTrackerBot::onCheckNearbyBots()
{

	TArray<AActor*> OverlapingBots;
	SphereNearBotComp->GetOverlappingActors(OverlapingBots);


	//if (DebugTrackerBotDrawing > 0) UE_LOG(LogTemp, Log, TEXT("Nearby bots: BotName= %s - NearbyBots=%s"), *GetName(), *FString::SanitizeFloat(OverlapingBots.Num()));

	int32 NrOfBots = OverlapingBots.Num();
	const int32 MaxPowerLevel = 4;
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	float Alpha = PowerLevel / (float)MaxPowerLevel;

	if (MatInst) {
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
			if (DebugTrackerBotDrawing > 0) DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");

		}
		else
		{
			// Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVeloctiyChange);

			if (DebugTrackerBotDrawing > 0) DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);

		}

		if (DebugTrackerBotDrawing > 0) DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{

	Super::NotifyActorBeginOverlap(OtherActor);

	if (bStartedSelfDestruction || bExploded)
	{
		return;
	}

	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn)
	{
		
		if (GetLocalRole() == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
		}

		bStartedSelfDestruction = true;
		// We overlapped with a player!

		// Start self destruction sequence
		

		UGameplayStatics::SpawnSoundAttached(SelfDesctructSound, RootComponent);

	}
}
