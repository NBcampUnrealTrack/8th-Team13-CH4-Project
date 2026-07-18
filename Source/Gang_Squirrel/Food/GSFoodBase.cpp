// Fill out your copyright notice in the Description page of Project Settings.

#include "GSFoodBase.h"

#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Gang_Squirrel/Food/GSFoodWidgetComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/Food/GSFoodWidget.h"
#include "Misc/OutputDeviceNull.h"

// Sets default values
AGSFoodBase::AGSFoodBase()
{
    bReplicates = true;
    SetReplicateMovement(true);
    
    USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
    RootComponent = DummyRoot;
    
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
    StaticMeshComponent->SetupAttachment(RootComponent); 
    
    FoodWidgetComponent = CreateDefaultSubobject<UGSFoodWidgetComponent>("FoodWidgetComponent");
    FoodWidgetComponent->SetupAttachment(RootComponent);
    
    SphereComponent = CreateDefaultSubobject<USphereComponent>("OverlapCollision");
    SphereComponent->SetupAttachment(RootComponent);
    
    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AGSFoodBase::OnBeginOverlap);
    SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AGSFoodBase::OnEndOverlap);
    
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGSFoodBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AGSFoodBase, bIsActive);
    DOREPLIFETIME(AGSFoodBase, FoodData);
}

void AGSFoodBase::BeginPlay()
{
    Super::BeginPlay();
   
    if (StaticMeshComponent)
    {
       StaticMeshComponent->SetRenderCustomDepth(true);
       StaticMeshComponent->SetCustomDepthStencilValue(1);
    }
    
    UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
    if (UserWidget)
    {
       UGSFoodWidget* FoodWidget = Cast<UGSFoodWidget>(UserWidget);
       if (FoodWidget)
       {
          CurrentFoodWidget = FoodWidget;
       }
    }
    
    if (FoodWidgetComponent)
    {
       if (FoodWidgetComponent->GetWidgetClass() == nullptr)
       {
          FString WidgetPath = TEXT("/Game/YJW/Food/FoodUI/WBP_FoodWidget.WBP_FoodWidget_C"); 
            
          UClass* LoadedClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, *WidgetPath);
          if (LoadedClass)
          {
             FoodWidgetComponent->SetWidgetClass(LoadedClass);
          }
       }
       
       FoodWidgetComponent->InitWidget();
       
       if (SphereComponent)
       {
          SphereComponent->SetRelativeScale3D(FVector(5.3f, 5.3f, 5.3f));
          SphereComponent->InitSphereRadius(1.f);
       }
    }
    
    bIsFilling = false;
    
    if (HasAuthority()) Activate();
}

void AGSFoodBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!IsValid(CurrentCharacter))
    {
       return;
    }
    
    if (bIsFilling && FoodWidgetComponent && CurrentCharacter->bIsEating && FoodData && CurrentCharacter->IsLocallyControlled())
    {
       CurrentEatenTime += DeltaTime;
       
       if (bIsStartEating && CurrentCharacter->bIsEating)
       {
          StartWidgetAnime();
          bIsStartEating = false;
       }
       
       float FinalTime = FoodData->EatTime * 4;
       
       float Alpha = FMath::Clamp(CurrentEatenTime / FinalTime, 0.f, 1.f );
       
       CurrentFoodWidget->UpdateProgress(Alpha);
       
       if (Alpha >= 0.190f)
       {
          bIsFilling = false;
          SetActorTickEnabled(false);
          
          StopWidgetAnim();
          
          if (IsValid(CurrentCharacter) && IsValid(FoodData))
          {
             CurrentCharacter->Server_AddTempScore(FoodData->ScoreAmount);
          }
          
          if (CurrentCharacter)
          {
             CurrentCharacter->Server_NotifyFoodEaten(this, CurrentCharacter);
          }
          
          
          Eaten(); 
       }
    }
    else
    {
       StopWidgetAnim();
       bIsStartEating = true;
    }
}

void AGSFoodBase::Init(UGSFoodPrimaryDataAsset* InData)
{
    if (!IsValid(InData)) return;
    FoodData = InData;
    
    if (!IsValid(StaticMeshComponent)) return;
    
    OnRep_FoodData();
}

void AGSFoodBase::OnRep_Activate()
{
   UE_LOG(LogTemp, Warning, TEXT("AGSFoodBase::OnRep_Activate"));
    this->SetActorHiddenInGame(!bIsActive);
    if (!bIsActive)
    {
    this->SetActorLocation(FVector(0.f,0.f, -1000000.f));
       UE_LOG(LogTemp, Warning, TEXT("%f"), this->GetActorLocation().Z);
       
    }
    this->SetActorEnableCollision(bIsActive);
}

void AGSFoodBase::Activate()
{
    if (!HasAuthority()) return;
    bIsActive = true;
    OnRep_Activate();
}

void AGSFoodBase::Deactivate()
{
    if (!HasAuthority()) return;
   
    
    SetActorTickEnabled(false);
   
    bIsActive = false;
    OnRep_Activate();
}

void AGSFoodBase::OnRep_FoodData() const
{
    if (!IsValid(FoodData)) return;
    
    StaticMeshComponent->SetStaticMesh(FoodData->FoodMesh);
    
    if (StaticMeshComponent)
    {
       float CurrentMeshSize = FoodData->MeshSize;
       StaticMeshComponent->SetRelativeScale3D(FVector(CurrentMeshSize,CurrentMeshSize,CurrentMeshSize));
       SphereComponent->SetSphereRadius(5.f);

       UStaticMeshComponent* MutableMeshComp = const_cast<UStaticMeshComponent*>(StaticMeshComponent.Get());
       if (MutableMeshComp)
       {
          UMaterialInterface* LoadedOverlayMat = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Game/ExternalContent/LevelPrototyping/Materials/M_Outline.M_Outline")));
          if (LoadedOverlayMat)
          {
             StaticMeshComponent->SetOverlayMaterial(LoadedOverlayMat);
          }
       }
    }
}

int32 AGSFoodBase::Eaten()
{
    Deactivate();
    return FoodData->ScoreAmount;
}

void AGSFoodBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AGSCharacter* OverlappedChar = Cast<AGSCharacter>(OtherActor);
    if (!OverlappedChar) return;

    // 이미 내가 이 음식을 제어하고 있다면 타인의 비빔 차단
    if (IsValid(CurrentCharacter) && CurrentCharacter->IsLocallyControlled())
    {
       return; 
    }

    if (OverlappedChar->IsLocallyControlled())
    {
       CurrentCharacter = OverlappedChar;
       
       bIsFilling = true;
       CurrentEatenTime = 0.f;
       
       SetActorTickEnabled(true);
       
       if (FoodWidgetComponent)
       {
          FoodWidgetComponent->SetVisibility(true);
          UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
          if (UserWidget)
          {
             FOutputDeviceNull ar;
             UserWidget->CallFunctionByNameWithArguments(TEXT("UpdateProgress 0.0"), ar, nullptr, true);
          }
       }
    }
}

void AGSFoodBase::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AGSCharacter* Character = Cast<AGSCharacter>(OtherActor);
    if (!Character) return;
    
    // 오직 먹고 있던 본인이 나갈 때만 리셋
    if (Character != CurrentCharacter)
    {
       return;
    }

    if (Character->IsLocallyControlled())
    {
       bIsFilling = false;
       CurrentEatenTime = 0.f;
       
       SetActorTickEnabled(false);
       
       if (FoodWidgetComponent)
       {
          FoodWidgetComponent->SetVisibility(false);
       }

       CurrentCharacter = nullptr;
    }
}

void AGSFoodBase::SetUIVisible(bool bShow)
{
    if (FoodWidgetComponent)
    {
       FoodWidgetComponent->SetVisibility(bShow);
    }
}

void AGSFoodBase::StartWidgetAnime()
{
    if  (FoodWidgetComponent)
    {
       CurrentFoodWidget->UpdateWidget();
    }
}

void AGSFoodBase::StopWidgetAnim()
{
    if  (FoodWidgetComponent)
    {
       CurrentFoodWidget->StopWidget();
    }
}

