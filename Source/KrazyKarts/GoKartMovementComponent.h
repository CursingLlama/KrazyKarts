// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() float Throttle;
	UPROPERTY() float SteeringThrow;
	UPROPERTY() float DeltaTime;
	UPROPERTY() float TimeStamp;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Sets default values for this component's properties
	UGoKartMovementComponent();

	UFUNCTION() void SimulateMove(const FGoKartMove& Move);
	UFUNCTION() FGoKartMove CreateMove(float DeltaTime);

	void SetThrottle(float Value) { Throttle = Value; }
	void SetSteeringThrow(float Value) { SteeringThrow = Value; }

	FVector GetVelocity() { return Velocity; }
	void SetVelocity(FVector Vector) { Velocity = Vector; }

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

private:	
		
	///Config
	//Mass of the Kart in Kilograms
	UPROPERTY(EditAnywhere) float Mass = 2000;
	//Horsepower of the Kart
	UPROPERTY(EditAnywhere) float Horsepower = 30;
	//Drag Coefficient for the Kart 
	UPROPERTY(EditAnywhere) float DragCoefficient = 16;
	//Rolling Coefficient for the Kart from friction 
	UPROPERTY(EditAnywhere) float RollingCoefficient = 0.015f;
	//Turning radius of the kart in meters
	UPROPERTY(EditAnywhere) float TurningRadius = 10;

	///State
	UPROPERTY() FVector Velocity;
	UPROPERTY() float Throttle;
	UPROPERTY() float SteeringThrow;

	///Functions

	void MoveKartForward(float DeltaTime, float Throttle);
	void RotateKart(float DeltaTime, float SteeringThrow);

	FVector GetAirResistance();
	FVector GetRollingResistance();

};
