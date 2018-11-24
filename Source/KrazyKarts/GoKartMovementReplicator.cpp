// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"

#include "UnrealNetwork.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();	
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MovementComponent) { return; }

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);
		UnacknowledgedMoves.Emplace(Move);
		Server_SendMove(Move);
	}

	//We are the server and in control of the pawn
	if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves()
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.TimeStamp > ServerState.LastMove.TimeStamp)
		{
			NewMoves.Emplace(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (!MovementComponent) { return; }

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves();

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (!MovementComponent) { return; }
	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	if (Move.Throttle > 1 || Move.Throttle < -1) return false;
	if (Move.SteeringThrow > 1 || Move.SteeringThrow < -1) return false;
	return true;
}
