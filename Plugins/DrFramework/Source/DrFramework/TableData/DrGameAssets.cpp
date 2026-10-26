#include "TableData/DrGameAssets.h"

#include "TableData/Generated/DrGameAssetsRow.h"

FSoftObjectPath DrGameAssets::Path(FName Key) {
  const FDrGameAssetsRow* Row = FDrGameAssetsRow::Find(Key);
  if (!Row || Row->Path.IsNull()) {
    UE_LOG(LogTemp, Warning, TEXT("GameAssets 테이블에 경로가 없습니다: %s"), *Key.ToString());
    return FSoftObjectPath();
  }
  return Row->Path;
}
