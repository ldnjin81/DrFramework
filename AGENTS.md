# DrFramework 작업 규칙 (AI 에이전트 공통)

- **구조 변경은 같은 작업에서 문서에 반영한다.** 클래스·모듈·플러그인 구성, 데이터 흐름, 테이블 스키마, 이름 규칙, 설정이 바뀌면 기반 변경은 `Docs/FRAMEWORK_FOR_AI.md`, 샘플 변경은 `Docs/SAMPLE_DESIGN.md`에 반영하고 양쪽 모두 `README.md`(사람용 개요)를 함께 고친다. 설계 명세서만으로 다른 엔진 버전에서 거의 같은 구조를 재현할 수 있어야 한다. 문서 변경이 빠진 구조 변경은 미완료다.
- 설계 명세서 파일 이름 `FRAMEWORK_FOR_AI.md`는 바꾸지 않는다(README의 다운로드 링크가 가리킨다). 릴리스마다 이 파일을 첨부한다.
- 장르를 제한하는 표현을 쓰지 않는다. 전투·보스 등은 "포함된 예시 구현"으로 표기한다.
- 프레임워크 클래스 접두사는 `Dr`, 샘플 모듈은 `Ds`. 플러그인에는 장르·샘플 타입을 넣지 않는다(샘플 없이 단독 빌드·테스트되어야 한다). 의존은 DrSample → DrFramework 한 방향.
- 기반 테스트(`DrFramework.*`)는 게임 데이터·다른 플러그인 없이 통과해야 한다.
- Epic 콘텐츠(Lyra·Game Animation Sample 등)는 저장소에 넣지 않는다. `Tools/import_epic_content.ps1`이 원래 패키지 경로(`Content/Characters` 등, gitignore)로 복사한다.
- DrFramework는 다른 프로젝트 플러그인(DrTableSystem 포함)·GAS에 의존하지 않는다. DrTableSystem은 권장 동반 플러그인이며 샘플이 서브모듈(`Plugins/DrTableSystem`)로 쓴다.
- `TableData/Generated`는 손으로 고치지 않는다. 스키마(`Source/DrSample/Tables`)를 고치고 `Tools/gen_tables.ps1`로 다시 만든다.
- 사람에게 보이는 글은 한국어로 쓴다.
