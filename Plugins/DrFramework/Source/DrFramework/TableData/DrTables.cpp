// DrFramework의 생성 테이블을 DrTable 플러그인 전역 레지스트리에 등록한다.
// 등록 코드(DrTableRegistration.h)는 Tools\generate_tables.cmd가 엑셀에서 생성한다.
#include "DrTableSubsystem.h"
#include "TableData/Generated/DrTableRegistration.h"

DRTABLE_AUTO_REGISTER(DrGeneratedTables::RegisterAll<UDrTableSubsystem>);
