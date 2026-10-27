# DrFramework

장르와 무관한 언리얼 엔진(UE 5.8) 게임 런타임 프레임워크 플러그인이다. 부팅 흐름·네트워크·가짜 서버·콘텐츠 등록·UI 흐름·검증 틀 같은 재사용 기반만 담는다. 다른 프로젝트 플러그인과 GAS에 의존하지 않으며(엔진 플러그인은 EnhancedInput·CommonUI만), 게임 모듈 없이 단독으로 빌드·테스트된다.

| 관련 저장소 | 관계 |
| --- | --- |
| [DrSample](https://github.com/ldnjin81/DrSample) | **샘플 프로젝트(참고 구현).** 이 플러그인 위에 근접 전투 게임 한 판을 만든 예시. 게임은 샘플을 포함하지 않고 참고만 한다 |
| [DrTableSystem](https://github.com/ldnjin81/DrTableSystem) | **권장 동반 플러그인.** 스키마 기반 정적 데이터 테이블. DrFramework 코드는 의존하지 않지만 함께 쓰기를 권장한다 |

## AI용 설계 명세서 내려받기

이 파일 하나만 AI 에이전트에 주면 다른 엔진 버전에서도 같은 기반 구조를 만들 수 있도록 구성했다. 실제 코드는 UE 5.8.2에서 검증했다.

- [최신(main) 명세서](https://raw.githubusercontent.com/ldnjin81/DrFramework/main/Docs/FRAMEWORK_FOR_AI.md): 링크를 열어 다른 이름으로 저장하면 바로 받는다.
- [버전별 명세서 — GitHub Releases](https://github.com/ldnjin81/DrFramework/releases): 각 릴리스에 `FRAMEWORK_FOR_AI.md`를 첨부한다.
- [저장소 안에서 읽기](Docs/FRAMEWORK_FOR_AI.md)
- 샘플 구현 명세: [샘플 설계 설명서](https://github.com/ldnjin81/DrSample/blob/main/Docs/SAMPLE_DESIGN.md)

## 구성

| 위치 | 역할 |
| --- | --- |
| `unreal/DrFramework` | 플러그인(접두사 `Dr`, `DRFRAMEWORK_API`) |
| `Host/DrFrameworkHost.uproject` | 단독 빌드·테스트용 호스트(코드 모듈 없음) |
| `Tools/verify.ps1` | 호스트 빌드 + `Automation RunTests DrFramework` |
| `Docs/FRAMEWORK_FOR_AI.md` | 설계 명세서 |
| `LICENSE` | MIT 라이선스 |

## 무엇을 제공하나

| 영역 | 기반 | 게임이 하는 일(샘플 예) |
| --- | --- | --- |
| 게임 인스턴스 | 부팅 단계 실행기(`GetBootSteps`·`ExecuteBootStep`·`CompleteBootStep`, 단계별 타임아웃) | 연결→Ping→로그인→프로필 |
| 게임 모드·설정 | `ADrGameMode`(폰 미지정), `UDrGameUserSettings`(`ApplyGameSettings` 훅) | 폰·컨트롤러 지정, 음량 저장 |
| 네트워크 | 요청 Id·콜백·타임아웃·지연 큐, 게임 정의 메시지 ID(FName) | 메시지 ID·페이로드 정의 |
| 가짜 서버 | `UDrFakeServer::RegisterHandler(Id, 처리기)`, 기본 Ping | 로그인·프로필 처리기 |
| 콘텐츠 | `UDrContentBase`·`UDrContentSubsystem`, `UDrContentSettings` 등록 목록, 서버 push 라우팅 | 프로필 콘텐츠 |
| UI | 화면 층(`UDrUIRootLayout`), 메뉴 동작(`UDrMenuWidget`), HUD 기반(`UDrHUDWidget`), `ADrPlayerController` 메뉴 스택·일시정지, `UDrUISettings` | 타이틀·일시정지·설정 메뉴, 전투 HUD |
| 검증 | 단계 등록형 스모크(`UDrSmokeSubsystem::RegisterStep`), 성능 표본 통계(`UDrPerfSubsystem`) | 스모크 단계·측정 경로 |

## 게임에 넣기

1. 플러그인을 게임의 `Plugins/DrFramework`에 둔다(서브모듈 `git submodule add https://github.com/ldnjin81/DrFramework Plugins/DrFramework` 또는 복사). UE가 `Plugins` 아래를 재귀로 찾으므로 `unreal/DrFramework/DrFramework.uplugin`이 인식된다.
2. 게임 모듈 `Build.cs`에 `DrFramework`를 추가한다. 정적 데이터는 DrTableSystem을 같은 방식으로 넣는 것을 권장한다.
3. 필요한 기반 클래스를 상속하고 ini에 지정한다. CommonUI 뷰포트(`CommonGameViewportClient`)를 쓴다. 메뉴·HUD 클래스와 입력은 프로젝트 설정 "DrFramework UI"(`UDrUISettings`)에서 정한다.
4. 장르 코드(전투·GAS·보스·UI 스타일 등)는 자기 모듈에 자기 접두사로 만든다. 방법은 [DrSample](https://github.com/ldnjin81/DrSample)을 참고한다.

## 검증

```powershell
.\Tools\verify.ps1      # 기본 엔진 경로: C:\Program Files\Epic Games\UE_5.8
```

UE 5.8.2 기준: 호스트 빌드 성공, `DrFramework.*` 6/6(네트워크 3, 가짜 서버 처리기 2, 스모크 결과 1). 샘플 위에서의 통합 검증(PIE 포함)은 DrSample 저장소에서 실행한다.

## 이름 규칙

- 클래스·구조체·열거형은 `Dr`, 테스트는 `DrFramework.*`. 게임은 자기 접두사를 쓴다(샘플 `Ds`).
- 기반은 게임플레이 태그·에셋 경로·메시지 목록·속성 세트를 정의하지 않는다.
- 구조가 바뀌면 같은 작업에서 설계 명세서와 이 README를 고친다([AGENTS.md](AGENTS.md)).

## 라이선스

[MIT](LICENSE). 이 저장소에는 Epic Games 콘텐츠가 들어 있지 않다. 샘플 프로젝트가 쓰는 Lyra 에셋은 각자 설치한 Lyra Starter Game에서 가져오며 Epic 콘텐츠 라이선스를 따른다.
