// Creative Commons - Reconocimiento (by)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystem;
class UDamageType;
class URadialForceComponent;



UCLASS()
class COOPGAME_API ASProjectile : public AActor
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ExplosionDamage;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;
	
	FTimerHandle TimeHandle_Boom;
	
	UFUNCTION()
	void ExecuteBoom();

	UPROPERTY(EditDefaultsOnly, BlueprintREadOnly, Category = "Weapon")
	UParticleSystem* BoomEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintREadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

public:	
	
	// Sets default values for this actor's properties
	ASProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }

	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }	
};
