// Creative Commons - Reconocimiento (by)

#include "../Public/ExplosiveBarrel.h"
#include "../Public/Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Materials/Material.h"
#include "DrawDebugHelpers.h"


static int32 DebugExplosiveBarrelDrawing = 0;
FAutoConsoleVariableRef CVARDebugExplosiveBarrelDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugExplosiveBarrelDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat
);

// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	ExplosionDamage = 80;
	ExplosionRadius = 400;
	
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("URadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bIgnoreOwningActor = true;
	RadialForceComp->bAutoActivate = false;
	ExplosionImpulse = 1000.0f;

	SetReplicates(true);
	SetReplicateMovement(true);

	
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);
}

// Called every frame
void AExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosiveBarrel::OnHealthChanged(USHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0.0f && !bExploded)
	{
		// Die!
		bExploded = true;
		
		UGameplayStatics::SpawnEmitterAtLocation(this, BoomFX, GetActorLocation());
		MeshComp->SetMaterial(0, ExplodeMaterial);

		FVector BosstIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BosstIntensity, NAME_None, true);

		RadialForceComp->FireImpulse();

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Apply Damage!
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
	
		if (DebugExplosiveBarrelDrawing > 0) DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 1);

	}
}

