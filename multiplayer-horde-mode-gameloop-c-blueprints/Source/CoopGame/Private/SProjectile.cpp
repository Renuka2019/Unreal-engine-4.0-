// Creative Commons - Reconocimiento (by)

#include "../Public/SProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "PhysicsEngine/RadialForceComponent.h"

static int32 DebugProjectileDrawing = 0;
FAutoConsoleVariableRef CVARDebugProjectileDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugProjectileDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat
);

void ASProjectile::ExecuteBoom()
{
	
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomEffect, GetActorLocation());
	
	TArray<AActor*> Actors;
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageType,Actors ,this,GetInstigatorController(),true);
	
	RadialForceComp->FireImpulse();

	SetLifeSpan(0.5f);

	if (DebugProjectileDrawing > 0) DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 1);

	
}


// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ASProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 20.0f;

	SetReplicates(true);
	SetReplicateMovement(true);

	ExplosionRadius = 200;
	ExplosionDamage = 80;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("URadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bIgnoreOwningActor = true;
	RadialForceComp->bAutoActivate = false;


}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().ClearTimer(TimeHandle_Boom);
	GetWorldTimerManager().SetTimer(TimeHandle_Boom, this, &ASProjectile::ExecuteBoom, 3.0f);
	
}

// Called every frame
void ASProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}
}
