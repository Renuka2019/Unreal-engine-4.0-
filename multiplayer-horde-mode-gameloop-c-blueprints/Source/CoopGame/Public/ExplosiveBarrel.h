// Creative Commons - Reconocimiento (by)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

class USHealthComponent;
class URadialForceComponent;

UCLASS()
class COOPGAME_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* BoomFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface* ExplodeMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ExplosionDamage;

	/* Barrel use previously */
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bExploded;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
