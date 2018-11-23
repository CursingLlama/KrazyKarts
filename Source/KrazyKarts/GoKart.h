// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"


USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() float Throttle;
	UPROPERTY() float SteeringThrow;
	UPROPERTY() float DeltaTime;
	UPROPERTY() float TimeStamp;
};

USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() FGoKartMove LastMove;
	UPROPERTY() FVector Velocity;
	UPROPERTY() FTransform Transform;
};

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	
	// Sets default values for this pawn's properties
	AGoKart();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MoveKartForward(float DeltaTime, float Throttle);
	void RotateKart(float DeltaTime, float SteeringThrow);
	FVector GetAirResistance();
	FVector GetRollingResistance();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	///Config
	//Mass of the Kart in Kilograms
	UPROPERTY(EditAnywhere) float Mass = 2000;
	//Horsepower of the Kart
	UPROPERTY(EditAnywhere) float Horsepower = 50;
	//Drag Coefficient for the Kart 
	UPROPERTY(EditAnywhere) float DragCoefficient = 16;
	//Rolling Coefficient for the Kart from friction 
	UPROPERTY(EditAnywhere) float RollingCoefficient = 0.015f;
	//Turning radius of the kart in meters
	UPROPERTY(EditAnywhere) float TurningRadius = 10;

	///State
	UPROPERTY() FVector Velocity;

	UPROPERTY(Replicated) float Throttle;
	UPROPERTY(Replicated) float SteeringThrow;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState) FGoKartState ServerState;
		
	///Functions
	UFUNCTION(Server, Reliable, WithValidation) void Server_SendMove(FGoKartMove Move);
	UFUNCTION() void SimulateMove(FGoKartMove Move);
	UFUNCTION() void OnRep_ServerState();

	void MoveForward(float Value);
	void TurnRight(float Value);

	FString GetRoleText(ENetRole Role);
};
