#pragma once
#include "CoreMinimal.h"
#include "UObject/UObjectHash.h"

namespace DrDebugSubsystems {
/**
 * 실제로 만들어질 파생 서브시스템이 있는지. 스모크·성능 기반은 파생이 있으면 자신을 만들지 않는다(같은 인자로 두 번 돌지 않게).
 * 추상 클래스와 ShouldCreateSubsystem이 false인 파생은 세지 않아, 그런 파생만 있으면 기반이 대신 만들어진다.
 */
inline bool HasCreatableDerived(const UClass* Base, UObject* Outer) {
  TArray<UClass*> Derived;
  GetDerivedClasses(Base, Derived, true);
  for (const UClass* Class : Derived) {
    if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) continue;
    if (const auto* Default = Cast<USubsystem>(Class->GetDefaultObject(false)); Default && Default->ShouldCreateSubsystem(Outer)) return true;
  }
  return false;
}
}
