# DrFramework 작업 규칙 (AI 에이전트 공통)

- **구조 변경은 같은 작업에서 문서에 반영한다.** 클래스·모듈·플러그인 구성, 데이터 흐름, 테이블 스키마, 이름 규칙, 설정이 바뀌면 `Docs/FRAMEWORK_FOR_AI.md`(설계 명세서)와 `README.md`(사람용 개요)를 함께 고친다. 설계 명세서만으로 다른 엔진 버전에서 거의 같은 구조를 재현할 수 있어야 한다. 문서 변경이 빠진 구조 변경은 미완료다.
- 설계 명세서 파일 이름 `FRAMEWORK_FOR_AI.md`는 바꾸지 않는다(README의 다운로드 링크가 가리킨다). 릴리스마다 이 파일을 첨부한다.
- 장르를 제한하는 표현을 쓰지 않는다. 전투·보스 등은 "포함된 예시 구현"으로 표기한다.
- 프레임워크 클래스 접두사는 `Dr`. 소비 게임 모듈은 자기 접두사를 쓴다.
- Epic 콘텐츠(Lyra·Game Animation Sample 등)는 저장소에 넣지 않는다. `Content/Epic/`(gitignore)으로 각자 복사한다.
- `TableData/Generated`는 손으로 고치지 않는다. 스키마(`Plugins/DrFramework/Tables`)를 고치고 `Tools/gen_tables.ps1`로 다시 만든다.
- 사람에게 보이는 글은 한국어로 쓴다.
