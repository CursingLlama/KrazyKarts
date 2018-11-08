// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	
	// Sets default values for this pawn's properties
	AGoKart();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MoveKartForward(float DeltaTime);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	///Config
	//Mass of the Kart in Kilograms
	UPROPERTY(EditAnywhere) float Mass = 1500;
	//Horsepower of the Kart
	UPROPERTY(EditAnywhere) float Horsepower = 50;

	///State
	UPROPERTY() FVector Velocity;
	UPROPERTY() FVector Force;
	
	///Functions
	UFUNCTION() void MoveForward(float Value);	
	
};
