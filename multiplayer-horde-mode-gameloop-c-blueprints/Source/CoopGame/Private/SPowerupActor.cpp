// Creative Commons - Reconocimiento (by)

#include "../CoopGame/Public/SPowerupActor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;
	bIsPowerupActive = false;

	SetReplicates(true);
}


void ASPowerupActor::OnTickPowerUp()
{
	TicksProcessed++;

	OnPowerupTicked();

	UE_LOG(LogTemp, Warning, TEXT("Element %s tick."), *GetName());

	if (TicksProcessed >= TotalNrOfTicks)
	{
		UE_LOG(LogTemp, Warning, TEXT("Element %s expired."), *GetName());
		OnExpired();
		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
	UE_LOG(LogTemp, Warning, TEXT("OnPowerupStateChanged %s activated."), *GetName());
}

void ASPowerupActor::ActivatedPowerup(AActor* ActiveFor)
{

	bIsPowerupActive = true;

	OnActivated(ActiveFor);

	UE_LOG(LogTemp, Warning, TEXT("Element %s activated."), *GetName());

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerUp, PowerupInterval, true);
	}
	else
	{
		OnTickPowerUp();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}


