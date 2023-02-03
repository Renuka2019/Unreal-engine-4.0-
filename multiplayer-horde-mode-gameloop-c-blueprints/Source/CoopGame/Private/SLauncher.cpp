// Creative Commons - Reconocimiento (by)

#include "../Public/SLauncher.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Public/SProjectile.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

void ASLauncher::Fire()
{

	if (GetLocalRole() < ROLE_Authority)
	{
		PlayFireEffects();
		ServerFire();
		return;
	}

	AActor* MyOwner = GetOwner();
	if (ProjectileClass && MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		// try and fire a projectile
		FVector MuzzleLocation = MeshComp->GetSocketLocation("MuzzleSocket");
		FRotator MuzzleRotation = EyeRotation;

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);

		LastFireTime = GetWorld()->TimeSeconds;

		if (GetLocalRole() == ROLE_Authority)
		{
			bShoot = FMath::FRand();
		}
	}
}

void ASLauncher::PlayFireEffects()
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);

	}
	/*
	APawn* MyPawnOwner = Cast<APawn>(GetOwner());
	if (MyPawnOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawnOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
	*/
}

void ASLauncher::OnRep_Shoot()
{
	PlayFireEffects();
}

void ASLauncher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASLauncher, bShoot, COND_SkipOwner);

}
