// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIBaseController.generated.h"



UCLASS()
class FORGOTTENVIGILANCE_API AAIBaseController : public AAIController
{
	GENERATED_BODY()

public:
	AAIBaseController();

	void StartBehaviorTree();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	virtual void BeginPlay() override;
};
