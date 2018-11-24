// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"
#include "GoKart.generated.h"

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
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	///State
	UPROPERTY(ReplicatedUsing = OnRep_ServerState) FGoKartState ServerState;
	UPROPERTY(EditAnywhere) UGoKartMovementComponent* MovementComponent = nullptr;

	TArray<FGoKartMove> UnacknowledgedMoves;
		
	///Functions
	UFUNCTION(Server, Reliable, WithValidation) void Server_SendMove(FGoKartMove Move);
	
	UFUNCTION() void ClearAcknowledgedMoves();
	
	UFUNCTION() void OnRep_ServerState();

	void MoveForward(float Value);
	void TurnRight(float Value);

	FString GetRoleText(ENetRole Role);
};
