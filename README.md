# DrFramework

언리얼 엔진 5.8용 3인칭 근접 액션 RPG 런타임 프레임워크와 샘플 프로젝트.

- `Plugins/DrFramework` — 프레임워크 플러그인(클래스 접두사 `Dr`). 게임 프로젝트는 이 플러그인을 넣고 필요한 곳만 상속한다.
- `Plugins/DrTableSystem` — 정적 데이터 테이블 플러그인([DrTableSystem](https://github.com/ldnjin81/DrTableSystem)).
- `Source/DrSample` — 프레임워크를 쓰는 최소 샘플 게임 모듈.
- `Docs/` — 설계 계약서(`FRAMEWORK_FOR_AI.md`)와 사람용 가이드(`FRAMEWORK_GUIDE.html`).

## 콘텐츠

저장소에는 코드와 직접 만든 데이터만 들어 있다. 마네킹·애니메이션 같은 Epic 콘텐츠는 라이선스상 공개 저장소에 다시 배포할 수 없으므로,
각자 설치한 Lyra Starter Game·Game Animation Sample에서 `Content/Epic/`으로 복사해 쓴다(복사 도구 준비 중).

## 상태

초기 구성 중. 원본 구현은 ProjectDT에서 옮겨 온다.
