# DrFramework 설계 명세서 — 언리얼 게임 런타임 프레임워크 (AI용, UE 5.8)

장르에 관계없이 게임의 수명·데이터·화면 흐름을 확장하는 코드 기반이다. 플러그인(저장소의 `unreal/DrFramework`, 게임에는 `Plugins/DrFramework` 아래에 둔다)은 소비 게임 모듈 없이 빌드할 수 있어야 한다. 게임 코드·스타일·에셋 경로·고정 메시지 목록·속성 세트는 포함하지 않는다.

## 샘플 프로젝트 (참고 구현)

이 기반 위에 실제 게임을 얹는 방법은 **[DrSample 샘플 프로젝트](https://github.com/ldnjin81/DrSample)**(접두사 `Ds`)가 보여 준다. 샘플은 기반 클래스를 상속·설정해 근접 전투 게임 한 판을 만든 예시이며, 명세는 [SAMPLE_DESIGN.md](https://github.com/ldnjin81/DrSample/blob/main/Docs/SAMPLE_DESIGN.md)(샘플 설계 설명서)에 있다.

- **참고만 하고 포함하지 않는다.** 소비 게임은 DrFramework 플러그인(정적 데이터가 필요하면 DrTableSystem 포함)만 넣고, 장르 코드는 자기 모듈에 자기 접두사로 둔다. 샘플 코드는 필요한 부분을 보고 따라 만들거나 복사해 출발점으로 삼는다.
- 샘플에서 볼 수 있는 것: 부팅 단계(연결→Ping→로그인→프로필), 가짜 서버 처리기·메시지 ID 정의, 콘텐츠 등록, 게임 흐름 훅(`OnEnterTitle`·`OnEnterGame`)에서 DrUIKit UI 흐름을 시작하는 법과 `UDrFlowSettings` 맵 설정, 스모크 단계 등록, DrTableSystem 테이블 생성·등록·굽기, GAS 상태 효과·어빌리티·속성 세트, 캐릭터·보스·엔카운터, Lyra 콘텐츠 가져오기와 PIE 검증.
- 기반 확장 지점과 샘플 구현의 대응은 SAMPLE_DESIGN.md의 "기반에서 확장한 지점" 표를 본다.
- 샘플 저장소는 DrFramework와 DrTableSystem을 git 서브모듈(`Plugins/DrFramework`, `Plugins/DrTableSystem`)로 가져온다. 게임도 같은 방식(서브모듈 또는 복사)으로 플러그인을 넣는다.

## 모듈 구조와 의존성

소비 게임 모듈 → DrFramework 방향만 허용하며, DrFramework는 다른 프로젝트 플러그인에 의존하지 않는다. 플러그인 클래스·구조체·열거형은 Dr 접두사, 공개 심볼은 DRFRAMEWORK_API를 쓴다. 모듈 루트를 PublicIncludePaths에 등록하고 다른 하위 폴더를 포함할 때 루트 상대 경로를 쓴다.

필수 엔진 모듈은 Core·CoreUObject·Engine·DeveloperSettings·InputCore·Json·Slate·SlateCore다. 엔진 플러그인을 요구하지 않는다(0.3.0부터 UI는 DrUIKit). GAS(GameplayAbilities)·게임플레이 태그는 쓰지 않는다. 실제 빌드 선언은 DrFramework.Build.cs가 기준이다. 엔진 상속 타입의 헤더와 각 cpp 자신의 헤더를 명시적으로 포함한다.

| 폴더 | 책임 |
| --- | --- |
| Game | 부팅 단계 실행기·게임 모드·게임 흐름 컨트롤러(타이틀·시작·복귀·재시작)·흐름 설정·사용자 설정 확장점 |
| Network | 끝점 연결 상태, 요청 Id·콜백·타임아웃·지연 큐 |
| Backend | 게임이 등록하는 ID별 처리기를 실행하는 가짜 서버 |
| Content | 콘텐츠 객체 등록·수명·서버 메시지 라우팅 |
| Debug | 단계 등록형 스모크, 성능 표본 통계 |

## 수명과 부팅

`UDrGameInstance::OnStart → StartBootFlow → GetBootSteps → ExecuteBootStep → CompleteBootStep` 순으로 진행한다. 단계는 순서가 있는 FName 배열이며 기본은 빈 배열이다. 단계 완료 성공이면 다음 단계, 실패이면 OnBootFinished(false)로 종료한다. 단계별 타임아웃은 기본 30초이며 게임이 변경할 수 있다. Shutdown에서 타이머와 실행 상태를 정리한다. 게임은 각 단계의 비동기 콜백에서 CompleteBootStep을 한 번만 호출해야 한다. 연결·인증·초기 데이터 요청은 게임 파생 클래스가 구현한다.

`ADrGameMode`는 DefaultPawnClass=nullptr로 시작하고 기반 컨트롤러를 지정한다. 게임이 사용할 폰·컨트롤러를 파생 게임 모드에서 선택한다. `UDrGameUserSettings`는 엔진 설정 저장 틀을 상속하고 ApplySettings 뒤 ApplyGameSettings 훅을 호출한다. 게임별 저장 항목과 적용은 게임 파생 클래스가 제공한다.

## 네트워크와 콘텐츠 데이터 흐름

`UDrNetworkSubsystem → UDrServerEndpoint → 응답 큐 → PendingRequests[RequestId] → FDrResponseDelegate`가 요청 경로다. FDrNetPacket.Type은 게임 정의 FName이며 고정 UENUM 목록을 두지 않는다. FInstancedStruct가 페이로드를 운반한다. 연결 상태는 Disconnected·Connecting·Connected·Reconnecting, 결과는 Ok·Timeout·Error·NotConnected·InvalidRequest다.

기본 끝점은 UDrFakeServer다. RegisterHandler(Id, TFunction)을 통해 메시지 처리기를 등록한다. 기본 Ping 왕복만 제공하며 알 수 없는 Id는 InvalidRequest를 반환한다. 서버와 클라이언트는 각각 지연 큐를 유지하고 ArrivalTime 순서를 보존한다. FDrLatencyProfile은 BaseLatencySeconds=.08, JitterSeconds=.03, FailureRate=0을 기본으로 한다. 요청 수명 기본값은 5초이며 만료 시 콜백을 한 번 호출한다. 연결 해제 시 대기 요청은 NotConnected로 정리한다. 세션 문자열은 게임이 SetSessionToken으로 설정하며 기반은 페이로드를 해석하지 않는다.

`UDrContentSettings.ContentClasses`(Config=Game)의 soft class 목록을 UDrContentSubsystem이 로드해 UDrContentBase 객체를 생성·Initialize한다. 게임은 RegisterContent로 추가 등록하거나 FindContent<T>로 조회한다. 서버 push는 RequestId=0 패킷이며 GetHandledMessageTypes에 Type이 포함된 콘텐츠만 HandleServerPacket을 받는다. 특정 사용자 데이터 콘텐츠는 기본 등록하지 않는다.

## 데이터 테이블: DrTableSystem 권장

DrFramework 코드는 테이블 시스템에 의존하지 않는다. 정적 데이터는 함께 쓰도록 권장하는 별도 플러그인 [DrTableSystem](https://github.com/ldnjin81/DrTableSystem)(스키마 → C++ 생성 → DataAsset 굽기 → `UDrTableSubsystem` 조회)으로 게임이 소유한다. 샘플은 DrTableSystem을 git 서브모듈(`Plugins/DrTableSystem`, 태그 고정)로 넣고 GameAssets(에셋 경로 표)·EffectDefs 등 모든 테이블을 자기 모듈에서 생성·등록한다. 게임은 자기 prefix로 생성하고, 자기 모듈 한 곳에서 `DRTABLE_AUTO_REGISTER`로 등록한다. drtable은 모듈 API 매크로를 붙이지 않으므로 다른 모듈이 행 정적 함수를 호출해야 하면 생성 직후 행 구조체에 모듈 API 매크로를 붙인다(샘플 `Tools/gen_tables.ps1` 참고).

## 게임 흐름과 UI(0.3.0)

UI(화면 층·메뉴·HUD·일시정지 메뉴·입력 모드·확인 대화상자)는 0.3.0부터 별도 플러그인 **DrUIKit**(`UDrUIFlowSubsystem`, 로컬 플레이어 서브시스템)이 맡는다. DrFramework는 UI 플러그인에 의존하지 않고 게임 흐름만 남긴다.

ADrPlayerController는 타이틀 판정(IsTitleScreen, TitleMap 기반)·StartGame(타이틀 → GameMap)·ReturnToTitle(일시정지 해제 후 TitleMap)·RequestRestart(BlueprintNativeEvent, 기본은 일시정지 해제)만 가진다. 로컬 컨트롤러의 BeginPlay에서 타이틀 맵이면 OnEnterTitle, 아니면 OnEnterGame을 부르므로(블루프린트는 On Enter Title·On Enter Game) 게임은 여기서 UI 흐름을 시작한다. 예: `UDrUIFlowSubsystem::Get(this)->EnterTitle()`·`EnterGame()`. 일시정지 메뉴는 DrUIKit 흐름이 컨트롤러의 SetPause만 불러 처리한다.

UDrFlowSettings는 UDeveloperSettings(Config=Game, DefaultConfig), 프로젝트 설정 표시 이름은 DrFramework 흐름이다. 값은 TitleMap·GameMap(TSoftObjectPtr<UWorld>) 둘이고 비어 있어도 허용한다.

| 컨트롤러 확장점 | 호출 시점 |
| --- | --- |
| OnEnterTitle / OnEnterGame | 로컬 컨트롤러 BeginPlay, 타이틀 맵 여부에 따라 하나 |
| OnPawnChanged | 소유 폰 변경, 초기 폰 포함 |
| RequestRestart (BlueprintNativeEvent) | 게임별 재시작; 기본은 일시정지 해제 |
| IsTitleScreen | TitleMap 설정 기반 판정, 필요 시 재정의 |

0.2.0에서 옮기기:

| 0.2.0 | 0.3.0 |
| --- | --- |
| UDrUIRootLayout (DrFramework) | DrUIKit UDrUIRootLayout(게임·메뉴·모달·알림 층, UI 흐름이 만든다) |
| UDrMenuWidget | DrUIKit UDrUIMenuWidget |
| UDrHUDWidget | DrUIKit UDrUIHUDWidget |
| UDrUISettings 메뉴·HUD 클래스, UIInputContext·PauseAction·MenuInputContext | DrUIKit UDrUIKitSettings '흐름'·'입력' |
| UDrUISettings TitleMap·GameMap | UDrFlowSettings |
| ADrPlayerController OpenPauseMenu·ClosePauseMenu·TogglePauseMenu·OpenSettings·BackMenu·IsMenuInput·IsPauseMenuOpen·GetHUDWidget·OnHUDCreated·OnMenuChanged·CanOpenPauseMenu | UDrUIFlowSubsystem OpenPauseMenu·ClosePauseMenu·TogglePauseMenu·OpenSettings·Back·IsMenuInput·IsPauseMenuOpen·GetHUD·OnMenuChanged·CanOpenPauseMenu |

블루프린트·에셋이 옛 클래스를 참조하면 게임 DefaultEngine.ini [CoreRedirects]에 `+ClassRedirects=(OldName="/Script/DrFramework.DrMenuWidget",NewName="/Script/DrUIKit.DrUIMenuWidget")`, `+ClassRedirects=(OldName="/Script/DrFramework.DrHUDWidget",NewName="/Script/DrUIKit.DrUIHUDWidget")`, `+ClassRedirects=(OldName="/Script/DrFramework.DrUIRootLayout",NewName="/Script/DrUIKit.DrUIRootLayout")`를 넣는다. DefaultGame.ini의 `[/Script/DrFramework.DrUISettings]` 값은 맵은 `[/Script/DrFramework.DrFlowSettings]`로, 나머지는 `[/Script/DrUIKit.DrUIKitSettings]`로 옮긴다.

## 검증 실행 틀

UDrSmokeSubsystem.RegisterStep은 이름(FString)·판정 콜백·타임아웃을 순서대로 등록한다. 이름은 결과 JSON 키로 대소문자 그대로, 등록 순서대로 쓰인다(FName은 엔진에 먼저 등록된 표기를 따르므로 키에 쓰지 않는다). 결과 JSON은 정적 함수 MakeResultJson으로 만든다. 콜백 결과는 Pending·Passed·Failed다. StartSmoke가 티커를 시작하고 실패/타임아웃 시 즉시 종료, 전부 성공 시 종료한다(종료 상태를 요청하지만 패키지 종료 코드는 엔진 종료 경로에 따라 0일 수 있으므로 판정은 result.json으로 한다). Saved/Smoke/result.json에 결과를 기록한다. 실행 인자 -DrSmokeTest가 있으면 Initialize가 가상 함수 RegisterSmokeSteps를 부른 뒤 StartSmoke를 부른다. 등록된 단계가 하나도 없으면 실패로 끝난다. 기반은 실제 게임 단계를 등록하지 않는다. 게임 파생이 RegisterSmokeSteps에서 시나리오를 구성한다. 만들어질 파생(추상이 아니고 ShouldCreateSubsystem이 true)이 있으면 기반 서브시스템은 만들어지지 않는다(Debug/DrDebugSubsystems.h, 스모크가 두 번 돌지 않게).

UDrPerfSubsystem은 측정 틀이다. 실행 인자 -DrPerfCapture가 있으면 티커를 시작해 ① PrepareCapture(World, OutError)가 true를 돌려줄 때까지 기다리고(PrepareTimeoutSeconds 기본 180초, OutError를 채우면 실패) ② 엔진 기본 품질(메모리에만)·stat unit·t.MaxFPS 0·r.VSync 0을 적용한 뒤 ③ WarmupSeconds(10초) 동안 UpdateCapture(World, INDEX_NONE, 0)을 부르고 ④ SectionNames 순서대로 구간마다 SectionSeconds(15초)씩 UpdateCapture(World, 구간, 진행률)를 부르며(UpdateCapture가 false면 카메라를 잃은 것으로 보고 실패) frame(틱 간격)·game·gpu(FStatUnitData Raw 값) 표본을 모은다 ⑤ Saved/Perf/result.json(error·duration_seconds·frame_budget_ms·scalability·width·height·completed·sections[name·frame·game_thread·gpu·over_budget·candidate_causes] + AddResultFields)을 쓰고 종료한다. 구간 p95 프레임이 FrameBudgetMs(16.7)를 넘으면 over_budget이고 OverBudgetHint를 candidate_causes로 남긴다. 예산 초과는 실패가 아니며, 표본 누락·구간 없음·갱신 실패·저장 실패는 completed=false다. Summarize는 표본을 정렬해 평균·ceil(N*.95)-1 인덱스의 p95·최댓값을 계산하고 빈 표본은 null이다. SectionAt(경과, 구간 길이, 구간 수)는 구간과 진행률을 계산한다(끝나면 false). 게임은 생성자에서 SectionNames 등을 정하고 준비·카메라 이동만 구현한다. 기반 생성 규칙은 스모크와 같다.

패키지 검증 도구(Tools/, Windows PowerShell, 공용 함수 _package_common.ps1, 인자 -Project <uproject> [-Out <폴더>, 기본 <프로젝트>/Saved/Package, 절대 경로로 바꿔 씀], Development·DebugGame만 — Shipping은 콘솔 명령·stat unit이 없고 Saved 위치가 달라 지원하지 않음, 판정은 종료 코드가 아니라 result.json): package_win64.ps1(RunUAT BuildCookRun, pak+iostore, -Maps 선택), smoke_win64.ps1(-DrSmokeTest로 실행, 이전 결과를 지우고 passed와 모든 단계를 검사, -Offscreen은 -RenderOffscreen), perf_win64.ps1(-DrPerfCapture, 해상도·completed 검사, 예산 초과는 경고), pso_bundle_win64.ps1(-logPSO로 -Runs 경로를 돌려 기록을 모으고(각 실행의 result.json이 실패면 중단) 쿡 결과의 셰이더 형식을 찾아(-ShaderFormat으로 지정 가능) ShaderPipelineCacheTools expand로 Build/Windows/PipelineCaches/PSO_<프로젝트>_<형식>.spc 생성. 전제: [DevOptions.Shaders] NeedsShaderStableKeys=true. 순서: 패키징 → PSO → 다시 패키징). 원격(SSH) 세션에서는 -Offscreen을 쓴다.

## 빈 프로젝트에서 재현

1. UE 5.8 프로젝트에 DrFramework를 넣고 활성화한다. UI가 필요하면 DrUIKit(CommonUI·EnhancedInput 사용)을 함께 넣는다. 정적 데이터가 필요하면 DrTableSystem도 함께 넣는다(권장). 전투·능력치가 필요하면 GAS는 게임이 직접 켠다(샘플 참고).
2. 소비 게임 모듈에 DrFramework를 의존성으로 추가한다. Target은 namespace 없이 BuildSettingsVersion.V7·EngineIncludeOrderVersion.Latest를 쓴다.
3. GameInstance·GameMode·GameUserSettings를 필요에 따라 상속하고 ini에 게임 클래스를 설정한다. DrUIKit을 쓰면 CommonGameViewportClient를 지정한다.
4. 맵 설정(DrFramework 흐름)과, DrUIKit을 쓰면 메뉴·HUD·입력 설정(DrUIKit '흐름'·'입력')을 필요한 값만 채운다. DrTableSystem을 쓰면 스키마 생성→게임 모듈 등록→빌드→굽기 순서다.
5. 플러그인 안에 소비 게임 include·타입·에셋 경로가 없는지 검사하고, DrSample 없이 빌드할 수 있음을 검증한다. 단독 검증은 저장소의 `Host/DrFrameworkHost.uproject`(코드 모듈 없음, `AdditionalPluginDirectories`로 `unreal`을 가리킴)를 `Build.bat UnrealEditor Win64 Development -Project=Host/DrFrameworkHost.uproject`로 빌드하고 `Automation RunTests DrFramework`를 실행한다. 기반 테스트는 게임 데이터·다른 플러그인 없이 통과해야 한다.
6. 에디터가 닫힌 환경에서 DrFramework와 게임 테스트를 실행한다(-ExecCmds에는 `Automation RunTests …` 하나만 쓴다. 앞에 `Automation List;`를 붙이면 0개가 실행된다). 외부 엔진 버전에서는 엔진 API 차이를 조정한다.

## 검증 명령과 문서 규약

`Tools/verify.ps1`이 호스트 빌드와 `Automation RunTests DrFramework`를 실행한다(헤드리스: `-ExecCmds="Automation RunTests DrFramework" -TestExit="Automation Test Queue Empty" -nullrhi -unattended`). 기반 테스트는 DrFramework.*(네트워크 3, 가짜 서버 처리기 2, 스모크 결과 1, 성능 구간·요약 2), 게임 테스트는 게임 모듈 이름을 쓴다(샘플은 DrSample.*, 샘플 저장소에서 실행). 기반 동작의 구조 변경은 이 파일과 README에 함께 반영하며, 게임 동작은 각 게임의 설계서(샘플은 SAMPLE_DESIGN.md)에 적는다. 이 파일 이름은 릴리스 다운로드 계약이므로 바꾸지 않는다.
