// Creative Commons - Reconocimiento (by)

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SLauncher.generated.h"

class ASProjectile;

/**
 * 
 */
UCLASS()
class COOPGAME_API ASLauncher : public ASWeapon
{
	GENERATED_BODY()
	
protected:
		virtual void Fire() override;

		void PlayFireEffects();

		UPROPERTY(ReplicatedUsing = OnRep_Shoot)
		float bShoot;

		UFUNCTION()
		void OnRep_Shoot();
				
public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<ASProjectile> ProjectileClass;
};
