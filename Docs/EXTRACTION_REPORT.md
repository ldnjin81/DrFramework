# DrFramework 1단계 추출 보고서 (작업 #63)

UE 5.8.2에서 DrSampleEditor Win64 Development 빌드 성공. 헤드리스 DrFramework 자동화 **64/64 Success, 실패 0**. ProjectDT 원본은 읽기만 했으며 쓰기 명령을 실행하지 않았다. 원본 저장소 상태 확인은 Claude가 수행한다(스레드 #825 보정). Git 커밋·push·브랜치 변경은 하지 않았으며 Claude가 codex/plugin-extract로 가져가 커밋한다(스레드 #828 보정).

## 파일과 경계

- DrFramework 소스 합계 141개: 런타임/Build.cs 110개, 테스트 13개, 생성 코드 18개.
- DrTableSystem: uplugin 1개 + Source 25개. 0.10.0 원본 Source와 바이트 비교 일치. 바이너리·Intermediate·exe는 복사하지 않았다(검증 빌드가 생성한 파일은 gitignore 대상).
- 스키마 4개·열거형 4개는 Plugins/DrFramework/Tables, 샘플 데이터 3개는 Design/Tables에 배치. 콘텐츠 파일(.uasset/.umap)은 생성·복사하지 않았다.
- DrSample 모듈은 모듈 등록만 한다. 런타임은 IMPLEMENT_MODULE(FDefaultModuleImpl, DrFramework)로 등록하고 DrTable은 한 진입점에서 자동 등록한다.
- Build.cs 원본 의존성 및 PublicIncludePaths.Add(ModuleDirectory) 유지. 프로젝트 플러그인 목록은 명세의 제외 규칙을 적용했고 실제 플러그인 의존성은 DrFramework.uplugin에 명시했다.

## 중립화와 데이터

- UDrBossData 기본 BossName=Boss, DisplayName=보스. 특정 보스 이름을 소스에서 제거했다.
- ADrBossCharacter.BossAssetKey: EditDefaultsOnly, Dr|Boss, 기본 Boss. BossData가 비었을 때 이 키로 조회한다.
- GameAssets.xlsx는 Player/PlayerCombat/CombatFX/Boss 네 키만 남기고 Path를 모두 비웠다. BossTables의 Boss 값도 Boss로 변경했다. 나머지 UI·사운드·몽타주 키는 런타임의 선택 조회 명세으로 유지되며 샘플에서는 누락 경고와 기존 대체 동작이 발생한다.
- DrTables의 스키마 참조 외부 링크는 ../../Plugins/DrFramework/Tables/Schemas로 바꿔 새 소유 경계를 반영했다. drtable build/check가 경고 없이 종료 0.
- Tools/gen_tables.ps1 -DrTable <0.10.0 실행 파일>이 --prefix Dr --ue-plugin으로 생성하고 참조 무결성을 검사한다. 기본값은 PC의 기존 실행 파일이며 새 실행 파일을 복사하거나 설치하지 않는다. DrTableBake는 수행하지 않았다.
- 샘플 DefaultGame.ini의 DrTableSettings.AssetRoot는 비웠다. 실제 게임은 구운 데이터 에셋 루트를 설정한다.

## 테스트 조정

- EffectDefs·BossPhase·BossAttack는 격리 UDrTableSubsystem와 메모리 행·정렬 기본키·CSR·배열 풀을 주입한다. TStrongObjectPtr가 수명을 보장하고 override는 픽스처 종료 시 해제한다. 실제 에셋을 굽거나 쓰지 않는다.
- DrUIMenuTest는 Engine.GameModeBase와 임시 URL을 사용해 GameAssets.Player의 게임 BP 로드를 분리했다. CommonInput의 실제 모듈 설정 인스턴스를 LoadData로 초기화해 BP 입력 데이터 없이 메뉴 규칙·슬라이더·설정 저장을 검증한다.
- 8방향 구르기는 방향 선택 검증을 유지한다. 몽타주 실물 검증만 분리했다. 보스 테이블도 행·패턴 참조·시간/거리 기준은 유지하고 애니메이션 실물 로드만 분리했다.
- -DtTreeQA/-DtArmorQA/-DtArenaQA/-DtPawnClass/-DtTryOutfit은 제외한 PIE 테스트에만 있어 새 런타임 플래그를 만들지 않았다. 이식된 런타임 플래그는 rename_map.csv에 기록했다.

## 제외하거나 부분 이식한 테스트

| 원본 파일 | 원본 테스트 이름 | 이유 |
| --- | --- | --- |
| Tests\DtArenaPIETest.cpp | ProjectDT.Arena.PIE.PropShots | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtAudioTest.cpp | ProjectDT.Audio.Assets.Coverage | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtAudioTest.cpp | ProjectDT.Audio.Mix.EncounterStateAndPhase | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtBossPIETest.cpp | ProjectDT.Boss.PIE.EngagesPlayer | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.InitializesAbilities | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.UsesGaspAndRetarget | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Animation.PawnInterface | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Movement.WalkRunSprint | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.JumpBlockedDuringCombatActions | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.GrantsCombatAbilities | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.HeroUsesMetaHuman | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.GreystoneUsesMetaHumanArmor | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCharactersTest.cpp | ProjectDT.Characters.Spawn.GameModeUsesGreystone | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtCombatPIETest.cpp | ProjectDT.Combat.PIE.InputSequence | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtEncounterPIETest.cpp | ProjectDT.Encounter.PIE.DeathRetryLoop | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Conversion.ReuseSystem | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Conversion.MeshMaterialsRestored | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Env.EffectTypeResumesCulled | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Data.AllBossAttacks | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Dash.ActivationAndCancellation | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Boss.TelegraphAndInterrupt | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Boss.PhaseAndRetry | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtFXTest.cpp | ProjectDT.FX.Boss.ImpactLightLifetime | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtGameAssetsTest.cpp | ProjectDT.Framework.Tables.GameAssets | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtInputLayoutTest.cpp | ProjectDT.Input.GamepadNoConflicts | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtLevelsTest.cpp | ProjectDT.Levels.BossArena.LoadMarkersAndNavigation | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtLevelsTest.cpp | ProjectDT.Levels.BossArena.MoodActors | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtPlaythroughPIETest.cpp | ProjectDT.Playthrough.PIE.EntryToBossDeathRetry | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtPlaythroughPIETest.cpp | ProjectDT.Playthrough.PIE.TitleReframed | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtUIPIETest.cpp | ProjectDT.UI.PIE.TitleStartTravel | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests\DtVisualPIETest.cpp | ProjectDT.Visual.PIE.MoodShots | PIE·맵·BP·시각/오디오·입력 에셋 의존 |
| Tests/DtFrameworkTablesTest.cpp | ProjectDT.Framework.Tables.BakedAsset | 구운 /Game/Data/DA_EffectDefs 에셋과 내용 해시 자체를 검증하므로 제외 |
| Tests/DtBossTest.cpp | ProjectDT.Boss.Tables.PhasesAndAttacksLoad | 행·인덱스는 메모리 주입으로 유지, 보스 애니메이션 실물 로드 검증만 소비 게임으로 보류 |
| Tests/DtCombatTest.cpp | ProjectDT.Combat.Dodge.RollDirections | 방향 선택 검증은 유지, 8방향 몽타주 실물 로드와 .73초 길이 검증은 콘텐츠 의존으로 보류 |


## ini에서 제외한 콘텐츠 경로

| 원본 파일 | 제외한 항목 |
| --- | --- |
| DefaultEngine.ini | `GameDefaultMap=/Game/Dt/Maps/L_Title` |
| DefaultGame.ini | `+MapsToCook=(FilePath="/Game/Dt/Maps/L_Title")` |
| DefaultGame.ini | `+MapsToCook=(FilePath="/Game/Dt/Maps/L_BossArena")` |
| DefaultGame.ini | `+DirectoriesToAlwaysCook=(Path="/Game/Dt/UI")` |
| DefaultGame.ini | `+DirectoriesToAlwaysCook=(Path="/Game/Dt/Input")` |
| DefaultGame.ini | `+DirectoriesToAlwaysCook=(Path="/Game/Dt/Audio")` |
| DefaultGame.ini | `InputData=/Game/Dt/UI/BP_UIInputData.BP_UIInputData_C` |
| DefaultGame.ini | `+ControllerData=/Game/Dt/UI/BP_KeyboardData.BP_KeyboardData_C` |
| DefaultGame.ini | `+ControllerData=/Game/Dt/UI/BP_GamepadData.BP_GamepadData_C` |

Android 파일 서버·로컬 HTTP/MCP·프로젝트 전용 CoreRedirects·프로젝트 ID·게임별 패키징·애니메이션 디버그 CVars는 샘플 필수 설정이 아니므로 복사하지 않았다. CommonUI viewport, EnhancedInput 클래스, 렌더링 기본 설정만 최소 구성에 유지했다.

## 이름 대응표

전체 103행은 [rename_map.csv](rename_map.csv)에 kind,old,new로 저장했다. class/struct/enum은 C++ 접두사까지 포함한 이름이다. CoreRedirects를 작성할 때 /Script/ProjectDT → /Script/DrFramework 모듈과 U/A/F/E 접두사 제거를 함께 적용해야 한다. 공용 State.*·Ability.*·Data.* 태그는 유지한다.

| 종류 | 이전 이름 | 새 이름 |
| --- | --- | --- |
| class | ADtBossCharacter | ADrBossCharacter |
| class | ADtCharacter | ADrCharacter |
| class | ADtCombatAnimPreview | ADrCombatAnimPreview |
| class | ADtGameMode | ADrGameMode |
| class | ADtPlayerController | ADrPlayerController |
| class | ADtTitleGameMode | ADrTitleGameMode |
| class | UDtAbilitySystemComponent | UDrAbilitySystemComponent |
| class | UDtAttributeSet | UDrAttributeSet |
| class | UDtAudioData | UDrAudioData |
| class | UDtAudioSubsystem | UDrAudioSubsystem |
| class | UDtBossAttackTable | UDrBossAttackTable |
| class | UDtBossBrainComponent | UDrBossBrainComponent |
| class | UDtBossData | UDrBossData |
| class | UDtBossPhaseTable | UDrBossPhaseTable |
| class | UDtCombatFXComponent | UDrCombatFXComponent |
| class | UDtCombatFXData | UDrCombatFXData |
| class | UDtCombatFeelData | UDrCombatFeelData |
| class | UDtCombatHUDViewModel | UDrCombatHUDViewModel |
| class | UDtCombatHUDWidget | UDrCombatHUDWidget |
| class | UDtContentBase | UDrContentBase |
| class | UDtContentSubsystem | UDrContentSubsystem |
| class | UDtEffectDefsTable | UDrEffectDefsTable |
| class | UDtEncounterSubsystem | UDrEncounterSubsystem |
| class | UDtFakeServer | UDrFakeServer |
| class | UDtGameAssetsTable | UDrGameAssetsTable |
| class | UDtGameInstance | UDrGameInstance |
| class | UDtGameUserSettings | UDrGameUserSettings |
| class | UDtGameplayAbility | UDrGameplayAbility |
| class | UDtGameplayAbility_ApplyEffect | UDrGameplayAbility_ApplyEffect |
| class | UDtGameplayAbility_AttackHeavy | UDrGameplayAbility_AttackHeavy |
| class | UDtGameplayAbility_AttackLight | UDrGameplayAbility_AttackLight |
| class | UDtGameplayAbility_Dodge | UDrGameplayAbility_Dodge |
| class | UDtGameplayAbility_Guard | UDrGameplayAbility_Guard |
| class | UDtGameplayAbility_MeleeAttack | UDrGameplayAbility_MeleeAttack |
| class | UDtGameplayEffect_Damage | UDrGameplayEffect_Damage |
| class | UDtGlassButton | UDrGlassButton |
| class | UDtGlassButtonStyle | UDrGlassButtonStyle |
| class | UDtHeavyHitShake | UDrHeavyHitShake |
| class | UDtHitFeedbackSubsystem | UDrHitFeedbackSubsystem |
| class | UDtLightHitShake | UDrLightHitShake |
| class | UDtLockOnComponent | UDrLockOnComponent |
| class | UDtLockOnTargetComponent | UDrLockOnTargetComponent |
| class | UDtMenuWidget | UDrMenuWidget |
| class | UDtNetworkSubsystem | UDrNetworkSubsystem |
| class | UDtPauseMenuWidget | UDrPauseMenuWidget |
| class | UDtPerfSubsystem | UDrPerfSubsystem |
| class | UDtPlayerCombatData | UDrPlayerCombatData |
| class | UDtPlayerProfileContent | UDrPlayerProfileContent |
| class | UDtServerEndpoint | UDrServerEndpoint |
| class | UDtSettingsMenuWidget | UDrSettingsMenuWidget |
| class | UDtSmokeSubsystem | UDrSmokeSubsystem |
| class | UDtTitleMenuWidget | UDrTitleMenuWidget |
| class | UDtUIRootLayout | UDrUIRootLayout |
| cmdline | -DtAutoNetSmoke | -DrAutoNetSmoke |
| cmdline | -DtCaptureAudio | -DrCaptureAudio |
| cmdline | -DtMenuCapture | -DrMenuCapture |
| cmdline | -DtPerfCapture | -DrPerfCapture |
| cmdline | -DtSmokeTest | -DrSmokeTest |
| console | Dt.Net.Ping | Dr.Net.Ping |
| console | Dt.Net.SetLatency | Dr.Net.SetLatency |
| console | Dt.Profile.Request | Dr.Profile.Request |
| console | Dt.Tables.Reload | Dr.Tables.Reload |
| enum | EDtAudioChannel | EDrAudioChannel |
| enum | EDtBossAttackKind | EDrBossAttackKind |
| enum | EDtBossState | EDrBossState |
| enum | EDtConnectionState | EDrConnectionState |
| enum | EDtEffectApplyTo | EDrEffectApplyTo |
| enum | EDtEffectCategory | EDrEffectCategory |
| enum | EDtEffectStackRule | EDrEffectStackRule |
| enum | EDtEncounterState | EDrEncounterState |
| enum | EDtHitWeight | EDrHitWeight |
| enum | EDtNetMessageType | EDrNetMessageType |
| enum | EDtNetResult | EDrNetResult |
| struct | FDtAttackFX | FDrAttackFX |
| struct | FDtBossAttackRow | FDrBossAttackRow |
| struct | FDtBossPhaseRow | FDrBossPhaseRow |
| struct | FDtDodgeTuning | FDrDodgeTuning |
| struct | FDtEffectDefsRow | FDrEffectDefsRow |
| struct | FDtEmptyPayload | FDrEmptyPayload |
| struct | FDtGameAssetsRow | FDrGameAssetsRow |
| struct | FDtGhostPose | FDrGhostPose |
| struct | FDtGuardTuning | FDrGuardTuning |
| struct | FDtHUDDamageTrail | FDrHUDDamageTrail |
| struct | FDtHitFeedbackState | FDrHitFeedbackState |
| struct | FDtHitFlashMesh | FDrHitFlashMesh |
| struct | FDtLatencyProfile | FDrLatencyProfile |
| struct | FDtLoginRequest | FDrLoginRequest |
| struct | FDtLoginResponse | FDrLoginResponse |
| struct | FDtMeleeStage | FDrMeleeStage |
| struct | FDtNetPacket | FDrNetPacket |
| struct | FDtNetResponse | FDrNetResponse |
| struct | FDtPendingRequest | FDrPendingRequest |
| struct | FDtPingPayload | FDrPingPayload |
| struct | FDtPlayerProfileDataSet | FDrPlayerProfileDataSet |
| struct | FDtQueuedClientPacket | FDrQueuedClientPacket |
| struct | FDtQueuedPacket | FDrQueuedPacket |
| struct | FDtResolvedEffect | FDrResolvedEffect |
| struct | FDtStatBlock | FDrStatBlock |
| struct | FDtStatFormula | FDrStatFormula |
| tag | Dt.Arena.Door | Dr.Arena.Door |
| tag | Dt.Boss | Dr.Boss |
| tag | Dt.Checkpoint.Gate | Dr.Checkpoint.Gate |
| tag | Dt.Player.Start | Dr.Player.Start |

## 검증 증거

```text
Build.bat DrSampleEditor Win64 Development -project=C:\Project\DrFramework\DrFramework.uproject -WaitMutex -UBARootDir=<TEMP>\UBA
Result: Succeeded
UnrealEditor-Cmd.exe DrFramework.uproject -ExecCmds="Automation List; RunTests DrFramework" -TestExit="Automation Test Queue Empty" -nullrhi -unattended
64 tests performed; Success 64, Fail 0
rg -n "Dt|PROJECTDT_API|/Script/ProjectDT|Sevarog" Plugins/DrFramework/Source
일치 0건 (rg 종료 1)
rg -n "/Game/" Plugins/DrFramework/Source
일치 0건
```

UBA의 action result store 저장 경고는 캐시 저장 경고이며 컴파일·링크 결과는 성공이다. 콘텐츠 없는 샘플의 GameAssets 누락·기본값 대체 경고는 예상된다. 전체 실행 로그는 PC TEMP의 task63-build-final.log, task63-tests-pass.log, 생성 JSON은 TEMP/DrFramework-DrTable에 있다. Agent Bridge 아티팩트 task-63-build/tests/diff에 요약을 올린다.

## 문서와 인계

- FRAMEWORK_FOR_AI.md와 루트 README.md에 Dr 플러그인/DrSample/소비 게임의 경계, 모듈·플러그인 의존성, 스키마·데이터 소유권, 클래스·콘솔·태그 접두사, 다른 엔진의 재현 순서를 반영했다.
- 구현상 새 설계 가정은 없다. 콘텐츠 의존 테스트 분리와 in-memory 주입은 명세가 허용한 방법이다.
- 제안 커밋 제목: `Extract ProjectDT runtime into DrFramework plugin`
- 추가 사용자 지시 #831–835 반영: HTML 가이드를 삭제하고 시스템별 개요·흐름·표·Mermaid 관계도를 한국어 README.md로 옮겼다. 콘텐츠 문단을 보존하고 최신/main·Releases 명세서 다운로드 링크를 소개 바로 아래에 배치했다. 소개와 플러그인 설명은 범용 프레임워크로 바꾸고 공통 기반과 예시 구현을 구분했다.
- 제안 커밋 파일: README.md, DrFramework.uproject, Source/, Plugins/DrFramework/, Plugins/DrTableSystem/{DrTableSystem.uplugin,Source/}, Config/, Design/Tables/, Tools/gen_tables.ps1, Docs/{FRAMEWORK_FOR_AI.md,EXTRACTION_REPORT.md,rename_map.csv}, Docs/FRAMEWORK_GUIDE.html 삭제.
- .git 변경·원격 push·Lore 명령은 실행하지 않았다. 커밋 해시는 Claude 대행 후 기록한다.

## 문서 리뷰 반영 (#838)

- README를 1,431줄에서 516줄로 요약했다. 중복된 플러그인 경계 표와 영어 병기 제목을 없애고 게임 연결 순서를 한 곳에 모았다.
- 관계 지도·상속도를 유지하고 붙은 라벨을 `<br/>`로 분리했다. 이벤트 연결과 주요 흐름을 Mermaid로 표현해 총 7개 그림을 제공한다.
- 정확한 시그니처·수치·테스트 목록은 설계 명세서의 해당 절로 연결했다. 최신/main·Releases 다운로드 링크와 Epic 콘텐츠 안내는 유지했다.
- README·설계 명세서·이 보고서의 기존 용어를 명세 또는 검증 기준으로 정리했다.
- 검증: README 7개 표의 앞뒤 빈 줄, 문서 3개의 코드 블록 닫힘·로컬 링크·명세서 앵커, Mermaid 선언·따옴표·괄호 구조, git diff --check 통과. Mermaid는 정적으로 점검했으며 GitHub 웹 화면에서의 실제 렌더링은 실행하지 않았다.
- 이번 재작업은 위 문서 3개만 수정했다. 런타임·테스트 코드는 그대로이므로 기존 빌드·64/64 성공 증거와 Claude의 #838 재검증 결과를 유지한다. ProjectDT에 쓰기를 수행하지 않았다.
- #828·#838 지시에 따라 커밋하지 않고 작업트리에 남긴다. 새 설계 가정은 없다.