# DrFramework 설계 명세서 — 언리얼 게임 런타임 프레임워크 (AI용, UE 5.8)

장르에 관계없이 게임의 수명·데이터·화면 흐름을 확장하는 코드 기반이다. 플러그인(저장소의 `unreal/DrFramework`, 게임에는 `Plugins/DrFramework` 아래에 둔다)은 소비 게임 모듈 없이 빌드할 수 있어야 한다. 게임 코드·스타일·에셋 경로·고정 메시지 목록·속성 세트는 포함하지 않는다.

## 샘플 프로젝트 (참고 구현)

이 기반 위에 실제 게임을 얹는 방법은 **[DrSample 샘플 프로젝트](https://github.com/ldnjin81/DrSample)**(접두사 `Ds`)가 보여 준다. 샘플은 기반 클래스를 상속·설정해 근접 전투 게임 한 판을 만든 예시이며, 명세는 [SAMPLE_DESIGN.md](https://github.com/ldnjin81/DrSample/blob/main/Docs/SAMPLE_DESIGN.md)(샘플 설계 설명서)에 있다.

- **참고만 하고 포함하지 않는다.** 소비 게임은 DrFramework 플러그인(정적 데이터가 필요하면 DrTableSystem 포함)만 넣고, 장르 코드는 자기 모듈에 자기 접두사로 둔다. 샘플 코드는 필요한 부분을 보고 따라 만들거나 복사해 출발점으로 삼는다.
- 샘플에서 볼 수 있는 것: 부팅 단계(연결→Ping→로그인→프로필), 가짜 서버 처리기·메시지 ID 정의, 콘텐츠 등록, 메뉴·HUD 파생과 `UDrUISettings` 설정, 스모크 단계 등록, DrTableSystem 테이블 생성·등록·굽기, GAS 상태 효과·어빌리티·속성 세트, 캐릭터·보스·엔카운터, Lyra 콘텐츠 가져오기와 PIE 검증.
- 기반 확장 지점과 샘플 구현의 대응은 SAMPLE_DESIGN.md의 "기반에서 확장한 지점" 표를 본다.
- 샘플 저장소는 DrFramework와 DrTableSystem을 git 서브모듈(`Plugins/DrFramework`, `Plugins/DrTableSystem`)로 가져온다. 게임도 같은 방식(서브모듈 또는 복사)으로 플러그인을 넣는다.

## 모듈 구조와 의존성

소비 게임 모듈 → DrFramework 방향만 허용하며, DrFramework는 다른 프로젝트 플러그인에 의존하지 않는다. 플러그인 클래스·구조체·열거형은 Dr 접두사, 공개 심볼은 DRFRAMEWORK_API를 쓴다. 모듈 루트를 PublicIncludePaths에 등록하고 다른 하위 폴더를 포함할 때 루트 상대 경로를 쓴다.

필수 엔진 모듈은 Core·CoreUObject·Engine·DeveloperSettings·EnhancedInput·InputCore·UMG·CommonUI·CommonInput·Json·Slate·SlateCore다. 엔진 플러그인은 EnhancedInput·CommonUI만 요구한다. GAS(GameplayAbilities)·게임플레이 태그는 쓰지 않는다. 실제 빌드 선언은 DrFramework.Build.cs가 기준이다. 엔진 상속 타입의 헤더와 각 cpp 자신의 헤더를 명시적으로 포함한다.

| 폴더 | 책임 |
| --- | --- |
| Game | 부팅 단계 실행기·게임 모드·UI 컨트롤러·사용자 설정 확장점 |
| Network | 끝점 연결 상태, 요청 Id·콜백·타임아웃·지연 큐 |
| Backend | 게임이 등록하는 ID별 처리기를 실행하는 가짜 서버 |
| Content | 콘텐츠 객체 등록·수명·서버 메시지 라우팅 |
| UI | 화면 층·메뉴 동작·HUD 기반·프로젝트 설정 페이지 |
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

## UI: 코드 기반과 설정

UDrUIRootLayout은 게임 층과 메뉴 층의 CommonActivatableWidgetStack을 가진다. PushGame·PushMenu·PopMenu·ClearMenus로 조작한다. UDrMenuWidget은 Abstract이며 Menu 입력 모드, 첫 포커스 FirstFocus, ESC/Gamepad_Special_Right 되돌아가기, UI 입력 매핑만 처리한다. 화면 레이아웃이나 문자열은 게임/BP가 만든다. UDrHUDWidget도 Abstract이며 Game 입력 모드와 OnOwnerPawnChanged 훅을 제공한다.

UDrUISettings는 UDeveloperSettings(Config=Game, DefaultConfig), 프로젝트 설정 표시 이름은 DrFramework UI다.

| 값 | 타입/의미 |
| --- | --- |
| TitleMenuClass·PauseMenuClass·SettingsMenuClass | TSoftClassPtr<UDrMenuWidget> |
| HUDClass | TSoftClassPtr<UDrHUDWidget> |
| UIInputContext | 메뉴 활성화 시 적용할 입력 매핑 |
| PauseAction | 일시정지 토글 액션 |
| MenuInputContext | 컨트롤러가 우선순위 100으로 등록할 매핑 |
| TitleMap·GameMap | 타이틀 판정·화면 전환용 TSoftObjectPtr<UWorld> |

모든 설정은 비어 있어도 허용한다. 클래스/맵이 없으면 해당 화면을 열지 않으며 임의 기본 메뉴·에셋을 로드하지 않는다. 메뉴 스택과 SetPause·입력 모드를 컨트롤러가 함께 관리한다. OpenPauseMenu는 타이틀/중복/CanOpenPauseMenu 조건을 확인하고 메뉴 클래스가 있을 때만 일시정지한다. Settings는 열린 메뉴에서만 추가한다. BackMenu는 설정을 먼저 닫고 이후 일시정지를 해제한다.

| 컨트롤러 확장점 | 호출 시점 |
| --- | --- |
| OnHUDCreated | 게임 층 초기화 뒤, nullrhi에서는 nullptr도 전달 |
| OnPawnChanged | 소유 폰 변경, 초기 폰 포함 |
| OnMenuChanged | 일시정지 메뉴 열기/닫기 |
| CanOpenPauseMenu | 게임별 메뉴 허용 조건 |
| RequestRestart (BlueprintNativeEvent) | 게임별 재시작; 기본은 메뉴 닫기 |
| IsTitleScreen | TitleMap 설정 기반 판정, 필요 시 재정의 |

## 검증 실행 틀

UDrSmokeSubsystem.RegisterStep은 이름(FString)·판정 콜백·타임아웃을 순서대로 등록한다. 이름은 결과 JSON 키로 대소문자 그대로, 등록 순서대로 쓰인다(FName은 엔진에 먼저 등록된 표기를 따르므로 키에 쓰지 않는다). 결과 JSON은 정적 함수 MakeResultJson으로 만든다. 콜백 결과는 Pending·Passed·Failed다. StartSmoke가 티커를 시작하고 실패/타임아웃 시 즉시 종료, 전부 성공 시 종료한다. Saved/Smoke/result.json에 결과를 기록한다. 기반은 실제 게임 단계를 등록하지 않는다. 게임 파생이 시나리오를 구성한다.

UDrPerfSubsystem은 구간별 frame/game/gpu 표본 저장소와 Summarize를 제공한다. 표본을 정렬하고 평균·ceil(N*.95)-1 인덱스의 p95·최댓값을 계산한다. 빈 표본의 수치는 null이다. 카메라·측정 시작 조건·구간 이름·파일 출력은 게임이 제공한다.

## 빈 프로젝트에서 재현

1. UE 5.8 프로젝트에 DrFramework를 넣고 활성화한다(CommonUI·EnhancedInput이 함께 켜진다). 정적 데이터가 필요하면 DrTableSystem도 함께 넣는다(권장). 전투·능력치가 필요하면 GAS는 게임이 직접 켠다(샘플 참고).
2. 소비 게임 모듈에 DrFramework를 의존성으로 추가한다. Target은 namespace 없이 BuildSettingsVersion.V7·EngineIncludeOrderVersion.Latest를 쓴다.
3. GameInstance·GameMode·GameUserSettings를 필요에 따라 상속하고 ini에 게임 클래스를 설정한다. CommonGameViewportClient를 지정한다.
4. UI 클래스·입력·맵 설정은 필요한 값만 채운다. DrTableSystem을 쓰면 스키마 생성→게임 모듈 등록→빌드→굽기 순서다.
5. 플러그인 안에 소비 게임 include·타입·에셋 경로가 없는지 검사하고, DrSample 없이 빌드할 수 있음을 검증한다. 단독 검증은 저장소의 `Host/DrFrameworkHost.uproject`(코드 모듈 없음, `AdditionalPluginDirectories`로 `unreal`을 가리킴)를 `Build.bat UnrealEditor Win64 Development -Project=Host/DrFrameworkHost.uproject`로 빌드하고 `Automation RunTests DrFramework`를 실행한다. 기반 테스트는 게임 데이터·다른 플러그인 없이 통과해야 한다.
6. 에디터가 닫힌 환경에서 Automation List 후 DrFramework와 게임 테스트를 실행한다. 외부 엔진 버전에서는 엔진 API 차이를 조정한다.

## 검증 명령과 문서 규약

`Tools/verify.ps1`이 호스트 빌드와 `Automation RunTests DrFramework`를 실행한다(헤드리스: `-ExecCmds="Automation RunTests DrFramework" -TestExit="Automation Test Queue Empty" -nullrhi -unattended`). 기반 테스트는 DrFramework.*(네트워크 3, 가짜 서버 처리기 2, 스모크 결과 1), 게임 테스트는 게임 모듈 이름을 쓴다(샘플은 DrSample.*, 샘플 저장소에서 실행). 기반 동작의 구조 변경은 이 파일과 README에 함께 반영하며, 게임 동작은 각 게임의 설계서(샘플은 SAMPLE_DESIGN.md)에 적는다. 이 파일 이름은 릴리스 다운로드 계약이므로 바꾸지 않는다.
