#include "gamescreen.h"

#include <prism/lifebar.h>
#include <prism/numberpopuphandler.h>
#include "storyscreen.h"

static struct 
{
    int mLevel = 0;
    int mGameTicks = 0;
    bool shownNarration[3];
    bool hasShownTutorial = false;
} gGameScreenData;

class GameScreen
{
public:
    double voiceLineVolume = 0.75;
    double sfxVolume = 0.1;

    GameScreen() {
        instantiateActor(getLifeBarHandler());
        instantiateActor(getPrismNumberPopupHandler());
        setVolume(0.1);
        load();
    }

    MugenSpriteFile mSprites;
    MugenAnimations mAnimations;
    MugenSounds mSounds;

    void loadFiles() {
        mSprites = loadMugenSpriteFileWithoutPalette("game/GAME.sff");
        mAnimations = loadMugenAnimationFile("game/GAME.air");
        mSounds = loadMugenSoundFile("game/GAME.snd");
    }

    enum class GameState
    {
        TITLE,
        TALK,
        INTRO1,
        INTRO2,
        TUTORIAL,
        FIGHT,
        KO,
        WIN,
        LOSE
    };
    GameState mGameState = GameState::TITLE;

    void load() {
        loadFiles();
        loadBG();
        loadSpeech();
        loadIntro();
        loadFight();
        loadWinning();
        loadLosing();
        loadKO();
        loadTitle();

        streamMusicFile((std::string("game/TALK") + getPlatformFilePath() + ".ogg").c_str());
        startTitle();
    }

    std::string getPlatformFilePath()
    {
        if (isOnDreamcast()) return "_DC";
        else return "";
    }

    int bgEntity = 0;
    void loadBG()
    {
        bgEntity = addBlitzEntity(Vector3D(0, 0, 1));
        addBlitzMugenAnimationComponent(bgEntity, &mSprites, &mAnimations, 1 + gGameScreenData.mLevel);
    }

    void update() {
        gGameScreenData.mGameTicks++;
        if (mGameState == GameState::TITLE)
        {
            updateTitle();
        }
        else if (mGameState == GameState::TALK)
        {
            updateSpeech();
        }
        else if (mGameState == GameState::INTRO1)
        {
            updateIntro1();
        }
        else if (mGameState == GameState::INTRO2)
        {
            updateIntro2();
        }
        else if (mGameState == GameState::TUTORIAL)
        {
            updateTutorial();
        }
        else if (mGameState == GameState::FIGHT)
        {
            updateFight();
        }
        else if (mGameState == GameState::KO)
        {
            updateKO();
        }
        else if (mGameState == GameState::WIN)
        {
            updateWinning();
        }
        else if (mGameState == GameState::LOSE)
        {
            updateLosing();
        }
    }

    // INTROS
    int introEntity;
    void loadIntro()
    {
        introEntity = addBlitzEntity(Vector3D(160, 120, 71));
        addBlitzMugenAnimationComponent(introEntity, &mSprites, &mAnimations, -1);
        setBlitzMugenAnimationVisibility(introEntity, false);
    }

    int introTicks = 0;
    void startIntro1()
    {
        loadLifebars();

        gGameScreenData.shownNarration[gGameScreenData.mLevel] = true;
        streamMusicFile((std::string("game/FIGHT") + getPlatformFilePath() + ".ogg").c_str());
        stopAllSoundEffects();
        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 300, 0, voiceLineVolume);
        }
        changeBlitzMugenAnimation(introEntity, 300);
        setBlitzMugenAnimationVisibility(introEntity, true);
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(0.0, 1.0));
        introTicks = 0;
        mGameState = GameState::INTRO1;
    }

    void endIntro1()
    {
        setBlitzMugenAnimationVisibility(introEntity, false);
    }

    double bounceLerp(double t) {
        if (t > 1.0) return 1.0;

        float bounce = sin(t * 3.5 * 3.14159) * (1.0 - t);
        float smooth = t + bounce * 0.3;

        return smooth;
    }

    double bounceLerp2(double t) {
        if (t > 1.0) return 1.0;

        float bounce = sin(t * 3.5 * 3.14159) * (1.0 - t);
        float smooth = t + bounce * 0.0;

        return smooth;
    }

    void updateIntro1()
    {
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(bounceLerp(introTicks / 120.0), 1.0));

        introTicks++;
        if (introTicks > 240 || (introTicks > 1 && (hasPressedStartFlank())))
        {
            endIntro1();
            startIntro2();
        }
    }

    void startIntro2()
    {
        stopAllSoundEffects();
        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 300, 1, voiceLineVolume);
        }
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(0.0, 1.0));
        changeBlitzMugenAnimation(introEntity, 310);
        setBlitzMugenAnimationVisibility(introEntity, true);
        introTicks = 0;
        mGameState = GameState::INTRO2;
    }

    void endIntro2()
    {
        setBlitzMugenAnimationVisibility(introEntity, false);
    }

    void updateIntro2()
    {
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(bounceLerp2(introTicks / 30.0), 1.0));
        introTicks++;
        if (introTicks > 60 || (introTicks > 1 && (hasPressedStartFlank())))
        {
            endIntro2();
            startTutorial();
        }
    }

    void startTutorial()
    {
        if (gGameScreenData.hasShownTutorial)
        {
            startFight();
            return;
        }

        stopAllSoundEffects();
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(1.0, 1.0));
        changeBlitzMugenAnimation(introEntity, 350);
        setBlitzMugenAnimationVisibility(introEntity, true);
        introTicks = 0;
        mGameState = GameState::TUTORIAL;
    }

    void endTutorial()
    {
        setBlitzMugenAnimationVisibility(introEntity, false);
        gGameScreenData.hasShownTutorial = true;
    }

    void updateTutorial()
    {
        introTicks++;
        if ((introTicks > 1 && (hasPressedStartFlank())))
        {
            endTutorial();
            startFight();
        }
    }

    // WINNING
    void loadKO()
    {

    }

    void startKO()
    {
        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 300, 2, voiceLineVolume);
        }
        clearAllBullets();
        changeBlitzMugenAnimation(introEntity, 340);
        setBlitzMugenAnimationVisibility(introEntity, true);
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(0.0, 1.0));

        if (playerHealth)
        {
            changeBlitzMugenAnimation(fightCharEntity, 10);
        }
        else
        {
            changeBlitzMugenAnimation(fightCharEntity, 14);
        }
        if (enemyHealth)
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo());
        }
        else
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 4);
        }

        introTicks = 0;
        mGameState = GameState::KO;
    }

    void endKO()
    {
        setBlitzMugenAnimationVisibility(introEntity, false);
    }

    void updateKO()
    {
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(bounceLerp2(introTicks / 45.0), 1.0));

        introTicks++;
        if (introTicks > 90 || (introTicks > 1 && (hasPressedStartFlank())))
        {
            endKO();
            if (enemyHealth == 0)
            {
                startWinning();
            }
            else
            {
                startLosing();
            }
        }
    }

    void loadWinning() {

    }

    void startWinning() {
        stopAllSoundEffects();
        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 300, 3, voiceLineVolume);
        }
        changeBlitzMugenAnimation(introEntity, 330);
        setBlitzMugenAnimationVisibility(introEntity, true);
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(0.0, 1.0));

        introTicks = 0;
        mGameState = GameState::WIN;
        stopStreamingMusicFile();
        tryPlayMugenSoundAdvanced(&mSounds, 100, 0, voiceLineVolume);
    }

    void endWinning() {
        setBlitzMugenAnimationVisibility(introEntity, false);
    }

    void updateWinning() {
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(bounceLerp(introTicks / 120.0), 1.0));

        introTicks++;
        if ((introTicks > 1 && (hasPressedStartFlank())))
        {
            endWinning();
            if (gGameScreenData.mLevel == 2)
            {
                setCurrentStoryDefinitionFile("game/OUTRO.def", 0);
                setNewScreen(getStoryScreen());
            }
            else
            {
                gGameScreenData.mLevel++;
                setNewScreen(getGameScreen());
            }
        }
    }

    void loadLosing() {
    
    }

    void startLosing() {
        stopAllSoundEffects();
        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 300, 4, voiceLineVolume);
        }
        changeBlitzMugenAnimation(introEntity, 320);
        setBlitzMugenAnimationVisibility(introEntity, true);
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(0.0, 1.0));

        introTicks = 0;
        mGameState = GameState::LOSE;
        stopStreamingMusicFile();
        tryPlayMugenSoundAdvanced(&mSounds, 100, 1, voiceLineVolume);
    }

    void endLosing() {
        setBlitzMugenAnimationVisibility(introEntity, false);
    }

    void updateLosing() {
        setBlitzMugenAnimationDrawScale(introEntity, Vector2D(bounceLerp(introTicks / 120.0), 1.0));

        introTicks++;
        if ((introTicks > 1 && (hasPressedStartFlank())))
        {
            endLosing();
            setNewScreen(getGameScreen());
        }
    }

    // TITLE
    enum class TitleStage
    {
        IDLE,
        ENDING
    };
    TitleStage mTitleStage = TitleStage::IDLE;
    int scrollBG1;
    int scrollBG2;
    int scrollBG3;
    int spinCharEntity;
    int kashaTitleEntity;
    void loadTitle()
    {
        scrollBG1 = addBlitzEntity(Vector3D(0, 0, 70));
        addBlitzMugenAnimationComponent(scrollBG1, &mSprites, &mAnimations, 210);
        setBlitzMugenAnimationVisibility(scrollBG1, false);

        scrollBG2 = addBlitzEntity(Vector3D(0, 240, 70));
        addBlitzMugenAnimationComponent(scrollBG2, &mSprites, &mAnimations, 210);
        setBlitzMugenAnimationVisibility(scrollBG2, false);

        scrollBG3 = addBlitzEntity(Vector3D(0, 480, 70));
        addBlitzMugenAnimationComponent(scrollBG3, &mSprites, &mAnimations, 1);
        setBlitzMugenAnimationVisibility(scrollBG3, false);

        spinCharEntity = addBlitzEntity(Vector3D(80, 140, 72));
        addBlitzMugenAnimationComponent(spinCharEntity, &mSprites, &mAnimations, 11);
        setBlitzMugenAnimationVisibility(spinCharEntity, false);

        kashaTitleEntity = addBlitzEntity(Vector3D(240, 300, 72));
        addBlitzMugenAnimationComponent(kashaTitleEntity, &mSprites, &mAnimations, 20);
        setBlitzMugenAnimationVisibility(kashaTitleEntity, false);
    }

    void startTitle()
    {
        if (gGameScreenData.mLevel != 0 || gGameScreenData.shownNarration[0])
        {
            startSpeech();
            return;
        }

        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 400, 0, voiceLineVolume);
        }
        setBlitzMugenAnimationVisibility(introEntity, true);
        changeBlitzMugenAnimation(introEntity, 200);
        introTicks = 0;
        mGameState = GameState::TITLE;
        mTitleStage = TitleStage::IDLE;
        
        setBlitzMugenAnimationVisibility(spinCharEntity, true);
        setBlitzMugenAnimationVisibility(scrollBG1, true);
        setBlitzMugenAnimationVisibility(scrollBG2, true);
        setBlitzMugenAnimationVisibility(scrollBG3, true);
    }

    void endTitle()
    {
        setBlitzMugenAnimationVisibility(introEntity, false);
        setBlitzMugenAnimationVisibility(spinCharEntity, false);

        setBlitzMugenAnimationVisibility(scrollBG1, false);
        setBlitzMugenAnimationVisibility(scrollBG2, false);
        setBlitzMugenAnimationVisibility(scrollBG3, false);
        setBlitzMugenAnimationVisibility(kashaTitleEntity, false);
    }

    void updateTitle() {
        if (mTitleStage == TitleStage::IDLE)
        {
            updateBgScrollingIdle();
            updateTitleIdle();
        }
        else
        {
            updateBgScrollingEnding();
            updateTitleEnding();
        }
    }

    void updateTitleIdle()
    {
        addBlitzEntityRotationZ(spinCharEntity, -0.4);
        if (hasPressedStartFlank())
        {
            stopAllSoundEffects();
            startTitleEnding();
        }
    }

    void startTitleEnding()
    {
        setBlitzMugenAnimationVisibility(introEntity, false);
        setBlitzMugenAnimationVisibility(scrollBG3, true);
        setBlitzMugenAnimationVisibility(kashaTitleEntity, true);
        auto* pos2 = getBlitzEntityPositionReference(scrollBG2);
        auto* pos3 = getBlitzEntityPositionReference(scrollBG3);
        auto* posKasha = getBlitzEntityPositionReference(kashaTitleEntity);
        pos3->y = pos2->y + 240;
        posKasha->y = pos2->y + 240 + 160;
        introTicks = 0;
        mTitleStage = TitleStage::ENDING;
        
    }


    void updateTitleEnding()
    {
        introTicks++;
        if (introTicks >= 240)
        {
            endTitle();
            startSpeech();
        }
    }

    void updateBgScrollingIdle()
    {
        auto* pos1 = getBlitzEntityPositionReference(scrollBG1);
        auto* pos2 = getBlitzEntityPositionReference(scrollBG2);
        pos1->y -= 16;
        pos2->y -= 16;
        if (pos1->y <= -240)
        {
            pos1->y += 480;
            std::swap(scrollBG1, scrollBG2);
        }
    }

    void updateBgScrollingEnding()
    {
        auto* pos1 = getBlitzEntityPositionReference(scrollBG1);
        auto* pos2 = getBlitzEntityPositionReference(scrollBG2);
        auto* pos3 = getBlitzEntityPositionReference(scrollBG3);
        auto* posKasha = getBlitzEntityPositionReference(kashaTitleEntity);
        if (pos3->y <= 0.0) return;

        pos1->y -= 16;
        pos2->y -= 16;
        pos3->y -= 16;
        posKasha->y -= 16;
        if (pos3->y <= 0.0)
        {
            tryPlayMugenSoundAdvanced(&mSounds, 2, 0, sfxVolume);
            pos3->y = 0.0;
        }
    }



    // Speech
    int textBoxEntity;
    int textBoxButtonEntity;
    int speechNameTextId;
    int speechTextId;
    int leftSpeakerAnimation = -1;
    int rightSpeakerAnimation = -1;
    void loadSpeech()
    {
        textBoxEntity = addBlitzEntity(Vector3D(160, 192, 51));
        addBlitzMugenAnimationComponent(textBoxEntity, &mSprites, &mAnimations, 500);
        setBlitzMugenAnimationVisibility(textBoxEntity, false);

        textBoxButtonEntity = addBlitzEntity(Vector3D(290, 230, 52));
        addBlitzMugenAnimationComponent(textBoxButtonEntity, &mSprites, &mAnimations, 502);
        setBlitzMugenAnimationVisibility(textBoxButtonEntity, false);

        leftSpeakerAnimation = addBlitzEntity(Vector3D(160, 0, 50));
        addBlitzMugenAnimationComponent(leftSpeakerAnimation, &mSprites, &mAnimations, 510);
        setBlitzMugenAnimationVisibility(leftSpeakerAnimation, false);

        rightSpeakerAnimation = addBlitzEntity(Vector3D(160, 0, 50));
        addBlitzMugenAnimationComponent(rightSpeakerAnimation, &mSprites, &mAnimations, gGameScreenData.mLevel == 2 ? 530 : 520);
        setBlitzMugenAnimationVisibility(rightSpeakerAnimation, false);
        setBlitzMugenAnimationFaceDirection(rightSpeakerAnimation, false);

        speechNameTextId = addMugenTextMugenStyle("", Vector3D(140, 165, 52), Vector3DI(2, 0, 1));
        setMugenTextVisibility(speechNameTextId, false);

        speechTextId = addMugenTextMugenStyle("", Vector3D(30, 190, 52), Vector3DI(1, 7, 1));
        setMugenTextVisibility(speechTextId, false);
        setMugenTextTextBoxWidth(speechTextId, 260);

        loadSpeechFile();
    }

    struct SpeechGroup
    {
        std::string speakerName;
        std::string text;
        std::string animleft;
        std::string animright;
    };
    std::vector<SpeechGroup> mLoadedSpeech;

    void loadSpeechFile()
    {
        MugenDefScript script;
        loadMugenDefScript(&script, std::string("game/SPEECH") + std::to_string(gGameScreenData.mLevel) + ".def");

        MugenDefScriptGroup* group = script.mFirstGroup;
        while (group)
        {
            SpeechGroup speech;
            speech.speakerName = getSTLMugenDefStringOrDefaultAsGroup(group, "name", "");
            speech.text = getSTLMugenDefStringOrDefaultAsGroup(group, "text", "");
            speech.animleft = getSTLMugenDefStringOrDefaultAsGroup(group, "leftanim", "");
            speech.animright = getSTLMugenDefStringOrDefaultAsGroup(group, "rightanim", "");
            mLoadedSpeech.push_back(speech);
            group = group->mNext;
        }

        unloadMugenDefScript(&script);
    }

    int currentSpeechStage = 0;
    void startSpeech()
    {
        if (gGameScreenData.shownNarration[gGameScreenData.mLevel])
        {
            startIntro1();
            return;
        }
        mGameState = GameState::TALK;
        currentSpeechStage = -1;
        loadNextSpeechStage();
    }

    int animNameToNumber(const std::string& name)
    {
        std::string copy = name;
        turnStringLowercase(copy);
        if (copy == "dante") return 510;
        else if (copy == "kasha") return 520;
        else if (copy == "broni") return 530;
        else if (copy == "yammer") return 540;
        else return 510;
    }

    void loadNextSpeechStage()
    {
        stopAllSoundEffects();
        currentSpeechStage++;
        if (currentSpeechStage >= mLoadedSpeech.size())
        {
            endSpeech();
            startIntro1();
            return;
        }

        if (!isOnDreamcast()) {
            tryPlayMugenSoundAdvanced(&mSounds, 200 + 10 * gGameScreenData.mLevel, currentSpeechStage, voiceLineVolume);
        }

        auto& newSpeech = mLoadedSpeech[currentSpeechStage];
        changeMugenText(speechNameTextId, newSpeech.speakerName.c_str());
        setMugenTextVisibility(speechNameTextId, true);
        changeMugenText(speechTextId, newSpeech.text.c_str());
        setMugenTextVisibility(speechTextId, true);
        setMugenTextBuildup(speechTextId, 1);

        if (newSpeech.animright != "")
        {
            setBlitzMugenAnimationFaceDirection(textBoxEntity, false);
            changeBlitzMugenAnimation(rightSpeakerAnimation, animNameToNumber(newSpeech.animright));
        }
        else if (newSpeech.animleft != "")
        {
            setBlitzMugenAnimationFaceDirection(textBoxEntity, true);
            changeBlitzMugenAnimation(leftSpeakerAnimation, animNameToNumber(newSpeech.animleft));
        }

        setBlitzMugenAnimationVisibility(textBoxEntity, true);
        setBlitzMugenAnimationVisibility(textBoxButtonEntity, false);
        setBlitzMugenAnimationVisibility(leftSpeakerAnimation, true);
        setBlitzMugenAnimationVisibility(rightSpeakerAnimation, true);
    }

    void endSpeech()
    {
        setMugenTextVisibility(speechNameTextId, false);
        setMugenTextVisibility(speechTextId, false);

        setBlitzMugenAnimationVisibility(textBoxEntity, false);
        setBlitzMugenAnimationVisibility(leftSpeakerAnimation, false);
        setBlitzMugenAnimationVisibility(rightSpeakerAnimation, false);
        setBlitzMugenAnimationVisibility(textBoxButtonEntity, false);
    }

    void updateSpeech()
    {
        updateSpeechButton();
        updateSpeechInput();
    }

    int speechButtonTicks = 0;
    void updateSpeechButton()
    {
        setBlitzMugenAnimationVisibility(textBoxButtonEntity, isMugenTextBuiltUp(speechTextId));
        setBlitzMugenAnimationPositionX(textBoxButtonEntity, sin(speechButtonTicks * 0.1) * 3);
        speechButtonTicks++;
    }

    void updateSpeechInput()
    {
        if (hasPressedAFlank())
        {
            if (isMugenTextBuiltUp(speechTextId))
            {
                loadNextSpeechStage();
            }
            else 
            {
                setMugenTextBuiltUp(speechTextId);
            }
        }
    }

    int getEnemyIdleAnim()
    {
        if (gGameScreenData.mLevel == 0)
        {
            return 20;
        } 
        else if (gGameScreenData.mLevel == 1)
        {
            return 30;
        }
        else
        {
            return 40;
        }
    }

    int getEnemyPortraitAnim()
    {
        if (gGameScreenData.mLevel == 0)
        {
            return 430;
        }
        else if (gGameScreenData.mLevel == 1)
        {
            return 431;
        }
        else
        {
            return 432;
        }
    }

    // FIGHT
    int fightCharEntity;
    int fightEnemyEntity;
    int fightCharShadowEntity;
    int fightEnemyShadowEntity;
    int slashEntity;
    int playerLifeBar;
    int enemyLifeBar;
    int playerLifeBarName;
    int enemyLifeBarName;
    int enemyPortrait;
    int playerHealth = 100;
    int enemyHealth = 100;
    void loadFight()
    {
        fightCharEntity = addBlitzEntity(Vector3D(60, 160, 20));
        addBlitzMugenAnimationComponent(fightCharEntity, &mSprites, &mAnimations, 10);

        fightCharShadowEntity = addBlitzEntity(Vector3D(60, 160, 19));
        addBlitzMugenAnimationComponent(fightCharShadowEntity, &mSprites, &mAnimations, 600);
        setBlitzMugenAnimationTransparency(fightCharShadowEntity, 0.3);

        slashEntity = addBlitzEntity(Vector3D(20, 200, 25));
        addBlitzMugenAnimationComponent(slashEntity, &mSprites, &mAnimations, -1);

        fightEnemyEntity = addBlitzEntity(Vector3D(260, 100, 20));
        addBlitzMugenAnimationComponent(fightEnemyEntity, &mSprites, &mAnimations, getEnemyIdleAnim());

        fightEnemyShadowEntity = addBlitzEntity(Vector3D(260, 100, 19));
        addBlitzMugenAnimationComponent(fightEnemyShadowEntity, &mSprites, &mAnimations, 600);
        setBlitzMugenAnimationTransparency(fightEnemyShadowEntity, 0.3);

        loadBullets();
    }

    void loadLifebars()
    {
        playerLifeBar = addLifeBar(Vector3D(10, 0, 30), mSprites, mAnimations, 420, 400, LifeBarType::STRETCH, 100, 100, 117, Vector3D(24, 13, 1));
        enemyLifeBar = addLifeBar(Vector3D(310, 0, 30), mSprites, mAnimations, 420, 410, LifeBarType::STRETCH, 100, 100, -117, Vector3D(-24, 13, 1));

        playerLifeBarName = addMugenTextMugenStyle("Dante", Vector3D(38, 10, 30), Vector3DI(2, 0, 1));
        enemyLifeBarName = addMugenTextMugenStyle(getEnemyNameString().c_str(), Vector3D(282, 10, 30), Vector3DI(2, 0, -1));

        enemyPortrait = addBlitzEntity(Vector3D(288, 2, 32));
        addBlitzMugenAnimationComponent(enemyPortrait, &mSprites, &mAnimations, getEnemyPortraitAnim());
    }

    std::string getEnemyNameString()
    {
        if (gGameScreenData.mLevel == 0) return "Kasha";
        else if (gGameScreenData.mLevel == 1) return "Broni";
        else return "Yammer";
    }

    void startFight()
    {
        mGameState = GameState::FIGHT;
    }

    void updateFight()
    {
        updateFightPlayer();
        updateFightEnemy();
        updateBullets();
    }

    void updateFightPlayer() {
        updateFightPlayerMovement();
        updateFightPlayerSlash();
        updateFightPlayerHitAnimation();
        updateFightPlayerShadow();
        updateFightPlayerZ();
    }

    void updateFightPlayerHitAnimation()
    {
        if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == 14 && !getBlitzMugenAnimationRemainingAnimationTime(fightCharEntity))
        {
            changeBlitzMugenAnimation(fightCharEntity, 10);
        }
    }

    void updateFightPlayerZ() {
        auto* pos = getBlitzEntityPositionReference(fightCharEntity);
        pos->z = (pos->y / 240.0) + 20;
    }

    void updateFightPlayerShadow()
    {
        auto* posChar = getBlitzEntityPositionReference(fightCharEntity);
        auto* posShadow = getBlitzEntityPositionReference(fightCharShadowEntity);
        *posShadow = *posChar;
        posShadow->z = (posShadow->y / 240.0) + 19;
    }

    void updateFightEnemyShadow()
    {
        auto* posChar = getBlitzEntityPositionReference(fightEnemyEntity);
        auto* posShadow = getBlitzEntityPositionReference(fightEnemyShadowEntity);
        *posShadow = *posChar;
        posShadow->z = (posShadow->y / 240.0) + 19;
    }
    void updateFightEnemyZ() {
        auto* pos = getBlitzEntityPositionReference(fightEnemyEntity);
        pos->z = (pos->y / 240.0) + 20;
    }

    Vector2DI lastDelta = Vector2DI(1, 0);
    void updateFightPlayerMovement() {
        Vector2DI delta = Vector2DI(0, 0);

        if(hasPressedLeft())
        {
            delta.x -= 1;
        }
        if(hasPressedRight())
        {
            delta.x += 1;
        }
        if(hasPressedUp())
        {
            delta.y -= 1;
        }
        if(hasPressedDown())
        {
            delta.y += 1;
        }
        if (delta.x != 0 || delta.y != 0)
        {
            auto* pos = getBlitzEntityPositionReference(fightCharEntity);
            *pos = *pos + delta * 2.0;
            *pos = clampPositionToGeoRectangle(*pos, GeoRectangle2D(48, 48, 224, 176));
            lastDelta = delta;
            if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == 10)
            {
                changeBlitzMugenAnimationIfDifferent(fightCharEntity, 12);
            }
            if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == 12)
            {
                setBlitzMugenAnimationFaceDirection(fightCharEntity, delta.x > 0);
            }

        }
        else
        {
            if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == 12)
            {
                changeBlitzMugenAnimationIfDifferent(fightCharEntity, 10);
            }
        }
    }

    void addPlayerDamage()
    {
        tryPlayMugenSoundAdvanced(&mSounds, 2, 2, sfxVolume * 2);
        playerHealth = std::max(0, playerHealth - 34);
        setLifeBarValue(playerLifeBar, playerHealth);
        changeBlitzMugenAnimation(fightCharEntity, 14);
        if (!playerHealth)
        {
            startKO();
        }
    }

    int getEnemyBaseAnimationNo()
    {
        if (gGameScreenData.mLevel == 0) return 20;
        else if (gGameScreenData.mLevel == 1) return 30;
        else return 40;
    }

    void updateFightPlayerSlash() {
        updateFightPlayerSlashStart();
        updateFightPlayerSlashAnimation();
        updateFightPlayerSlashPosition();
        updateFightPlayerSlashHit();
    }

    void updateFightPlayerSlashAnimation()
    {
        if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == 13 && !getBlitzMugenAnimationRemainingAnimationTime(fightCharEntity))
        {
            changeBlitzMugenAnimation(fightCharEntity, 10);
        }
    }

    Vector2D lastSlashOffset = Vector2D(0, 0);
    void updateFightPlayerSlashStart() {
        if (hasPressedAFlank())
        {
            double angle = getAngleFromDirection(Vector2D(lastDelta.x, lastDelta.y));
            auto charPos = getBlitzEntityPosition(fightCharEntity);
            auto testCenter = charPos.xy() - Vector2DI(0, 12);
            changeBlitzMugenAnimation(slashEntity, 100);
            changeBlitzMugenAnimation(fightCharEntity, 13);
            setBlitzEntityPosition(slashEntity, testCenter.xyz(25));
            setBlitzMugenAnimationAngle(slashEntity, angle + M_PI);
            tryPlayMugenSoundAdvanced(&mSounds, 2, 5, sfxVolume);
        }
    }

    void updateFightPlayerSlashPosition()
    {
        auto charPos = getBlitzEntityPosition(fightCharEntity);
        auto testCenter = charPos.xy() - Vector2DI(0, 12);
        setBlitzEntityPosition(slashEntity, testCenter.xyz(25));
    }

    void checkSlashOnBullet()
    {
        for (auto& bp : mBullets)
        {
            auto& b = bp.second;
            auto charPos = getBlitzEntityPosition(fightCharEntity) - Vector2DI(0, 12);
            auto enemyPos = getBlitzEntityPosition(b.entityId);
            auto testCenter = charPos.xy() + (lastDelta * 25);
            auto enemyDist = vecLength(enemyPos.xy() - testCenter);
            if (enemyDist < b.radius)
            {
                auto dirNorm = vecNormalize(b.direction);
                auto deltNorm = vecNormalize(charPos - enemyPos);
                bool hasRebounded = false;
                if ((dirNorm.x < 0 && deltNorm.x < 0) || (dirNorm.x > 0 && deltNorm.x > 0))
                {
                    b.direction.x = -b.direction.x;
                    hasRebounded = true;
                }
                if ((dirNorm.y < 0 && deltNorm.y < 0) || (dirNorm.y > 0 && deltNorm.y > 0))
                {
                    b.direction.y = -b.direction.y;
                    hasRebounded = true;
                }
                if (hasRebounded)
                {
                    b.direction = 2.0 * b.direction;
                }
            }
        }
    }

    void updateFightPlayerSlashHit() {
        if (getBlitzMugenAnimationAnimationNumber(slashEntity) == 100 && getBlitzMugenAnimationAnimationStep(slashEntity) == 1)
        {
            checkSlashOnBullet();
            if (doesSlashConnectWithEnemy())
            {
                damageEnemy();
            }
        }
    }

    bool doesSlashConnectWithEnemy()
    {
        auto charPos = getBlitzEntityPosition(fightCharEntity);
        auto enemyPos = getBlitzEntityPosition(fightEnemyEntity) - Vector2DI(0, 12);
        auto testCenter = (charPos.xy() - Vector2DI(0, 12)) + (lastDelta * 25);
        auto enemyDist = vecLength(enemyPos.xy() - testCenter);
        return enemyDist < 20;
    }

    void damageEnemy()
    {
        tryPlayMugenSoundAdvanced(&mSounds, 2, 2, sfxVolume);
        auto enemyPos = getBlitzEntityPosition(fightEnemyEntity) - Vector2DI(0, 32);
        addPrismNumberPopup(10, enemyPos.xy().xyz(28), 1, Vector3D(0, -0.5, 0), 1.0, COLOR_RED, 30);
        enemyHealth = std::max(0, enemyHealth - 2);
        setLifeBarValue(enemyLifeBar, enemyHealth);
        changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 4);
        if (!enemyHealth)
        {
            startKO();
        }
    }

    enum class EnemyState
    {
        SHOOT,
        WAIT,
        MOVEMENT
    };
    EnemyState mEnemyState = EnemyState::WAIT;
    int mEnemyWaitTicks = 180;

    void updateFightEnemy() {
        updateFightEnemyCurrentState();
        updateFightEnemyGetHitAnimation();
        updateFightEnemyShadow();
        updateFightEnemyZ();
    }

    void updateFightEnemyGetHitAnimation()
    {
        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == (getEnemyBaseAnimationNo() + 4) && !getBlitzMugenAnimationRemainingAnimationTime(fightEnemyEntity))
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo());
        }
    }

    void updateFightEnemyCurrentState()
    {
        if (mEnemyState == EnemyState::SHOOT)
        {
            updateFightEnemyShoot();
        }
        else if (mEnemyState == EnemyState::MOVEMENT)
        {
            updateFightEnemyMovement();
        }
        else
        {
            updateFightEnemyWait();
        }
    }

    Vector2D target = Vector2D(160, 120);
    void startFightEnemyMovement()
    {
        changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo());
        mEnemyState = EnemyState::MOVEMENT;
        target = Vector2D(randfrom(48, 270), randfrom(64, 222));
        broniMoveTicks = -30;
    }

    void updateFightEnemyMovementKasha()
    {
        if (broniMoveTicks < 0 && (abs(broniMoveTicks) % 10) == 0)
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 0.0, 0.0);
        }
        else
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 1.0, 1.0);
        }
        broniMoveTicks++;
        if (broniMoveTicks < 0) return;

        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == getEnemyBaseAnimationNo())
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 2);
        }

        auto pos = getBlitzEntityPosition(fightEnemyEntity).xy();
        auto dist = vecLength(target - pos);
        auto dir = vecNormalize(target - pos);
        if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == getEnemyBaseAnimationNo() + 2)
        {
            setBlitzMugenAnimationFaceDirection(fightCharEntity, dir.x > 0);
        }
        pos = pos + dir * 8.0;
        if (dist <= 8.0 + 1)
        {
            setBlitzEntityPosition(fightEnemyEntity, target.xyz(getBlitzEntityPosition(fightEnemyEntity).z));
            startFightEnemyWait(30);
        }
        else
        {
            setBlitzEntityPosition(fightEnemyEntity, pos.xyz(getBlitzEntityPosition(fightEnemyEntity).z));
        }
    }

    int broniMoveTicks = 0;
    void updateFightEnemyMovementBroni()
    {
        if (broniMoveTicks < 0 && (abs(broniMoveTicks) % 10) == 0)
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 0.0, 0.0);
        }
        else
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 1.0, 1.0);
        }
        broniMoveTicks++;
        if (broniMoveTicks < 0) return;

        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == getEnemyBaseAnimationNo())
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 2);
        }
        auto pos = getBlitzEntityPosition(fightEnemyEntity).xy();
        auto dist = vecLength(target - pos);
        auto dir = vecNormalize(target - pos);
        if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == getEnemyBaseAnimationNo() + 2)
        {
            setBlitzMugenAnimationFaceDirection(fightCharEntity, dir.x > 0);
        }
        pos = pos + dir * 2.0;
        if (broniMoveTicks > 20)
        {
            addGiantAimedShot();
            broniMoveTicks = 0;
        }
        if (dist <= 2.0 + 1)
        {
            setBlitzEntityPosition(fightEnemyEntity, target.xyz(getBlitzEntityPosition(fightEnemyEntity).z));
            startFightEnemyWait(180);
        }
        else
        {
            setBlitzEntityPosition(fightEnemyEntity, pos.xyz(getBlitzEntityPosition(fightEnemyEntity).z));
        }
    }

    void updateFightEnemyMovementYammer()
    {
        if (broniMoveTicks < 0 && (abs(broniMoveTicks) % 10) == 0)
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 0.0, 0.0);
        }
        else
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 1.0, 1.0);
        }
        broniMoveTicks++;
        if (broniMoveTicks < 0) return;

        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == getEnemyBaseAnimationNo())
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 2);
        }
        auto pos = getBlitzEntityPosition(fightEnemyEntity).xy();
        auto dist = vecLength(target - pos);
        auto dir = vecNormalize(target - pos);
        if (getBlitzMugenAnimationAnimationNumber(fightCharEntity) == getEnemyBaseAnimationNo() + 2)
        {
            setBlitzMugenAnimationFaceDirection(fightCharEntity, dir.x > 0);
        }
        pos = pos + dir * 8.0;
        if (broniMoveTicks > 10)
        {
            addGiantAimedShot();
            broniMoveTicks = 0;
        }
        if (dist <= 8.0 + 1)
        {
            setBlitzEntityPosition(fightEnemyEntity, target.xyz(getBlitzEntityPosition(fightEnemyEntity).z));
            startFightEnemyWait(180);
        }
        else
        {
            setBlitzEntityPosition(fightEnemyEntity, pos.xyz(getBlitzEntityPosition(fightEnemyEntity).z));
        }
    }

    void startFightEnemyWait(int duration)
    {
        changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo());
        mEnemyWaitTicks = duration;
        mEnemyState = EnemyState::WAIT;
    }

    void updateFightEnemyWaitKasha()
    {
        mEnemyWaitTicks--;
        if (mEnemyWaitTicks <= 0)
        {
            auto rand = randfromInteger(0, 2);
            if (!rand)
            {
                startFightEnemyShoot();
            }
            else
            {
                startFightEnemyMovement();
            }
        }
    }

    void updateFightEnemyWaitBroni()
    {
        mEnemyWaitTicks--;
        if (mEnemyWaitTicks <= 0)
        {
            auto rand = randfromInteger(0, 100);
            if (rand < 10)
            {
                startFightEnemyShoot();
            }
            else
            {
                startFightEnemyMovement();
            }
        }
    }

    void updateFightEnemyWaitYammer()
    {
        mEnemyWaitTicks--;
        if (mEnemyWaitTicks <= 0)
        {
            auto rand = randfromInteger(0, 100);
            if (rand < 70)
            {
                startFightEnemyShoot();
            }
            else
            {
                startFightEnemyMovement();
            }
        }
    }

    int shootTicks = 0;
    int shotPattern = 0;
    void startFightEnemyShoot()
    {
        changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo());
        shootTicks = -30;
        shotPattern = randfromInteger(0, 100);
        mEnemyState = EnemyState::SHOOT;
    }

    void addRandomShot()
    {
        double angle = randfrom(0, 2 * M_PI);
        addAngledShot(angle);
    }

    void addAngledShot(double angle)
    {
        auto enemyPos = getBlitzEntityPosition(fightEnemyEntity).xy() - Vector2DI(0, 12);
        Vector2D direction = vecRotateZ2D(Vector2D(1, 0), angle);
        double speed = randfrom(2.0, 3.0);
        addBullet(enemyPos + direction * 10, direction * speed, 120, 5);
    }

    void addGiantAimedShot()
    {
        auto enemyPos = getBlitzEntityPosition(fightEnemyEntity).xy() - Vector2DI(0, 12);
        auto charPos = getBlitzEntityPosition(fightCharEntity).xy() - Vector2DI(0, 12);
        auto delt = vecNormalize(enemyPos - charPos);
        double angle = getAngleFromDirection(delt);
        Vector2D direction = vecRotateZ2D(Vector2D(1, 0), M_PI + M_PI - angle);
        double speed = 1.0;
        addBullet(enemyPos + direction * 10, direction * speed, 121, 9);
    }

    void updateFightEnemyShootKasha()
    {
        if (shootTicks < 0 && (abs(shootTicks) % 10) == 0)
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 0.0, 0.0);
        }
        else
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 1.0, 1.0);
        }

        shootTicks++;
        if (shootTicks < 0) return;
        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == getEnemyBaseAnimationNo())
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 3);
        }
        if (shootTicks % 10 == 0)
        {
            addRandomShot();
        }
        if (shootTicks >= 360)
        {
            startFightEnemyWait(150);
        }
    }

    void updateFightEnemyShootBroni()
    {
        if (shootTicks < 0 && (abs(shootTicks) % 10) == 0)
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 0.0, 0.0);
        }
        else
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 1.0, 1.0);
        }
        shootTicks++;
        if (shootTicks < 0) return;

        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == getEnemyBaseAnimationNo())
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 3);
        }
        if (shootTicks % 10 == 0)
        {
            addRandomShot();
        }
        if (shootTicks >= 180)
        {
            startFightEnemyWait(60);
        }
    }

    void updateFightEnemyShootYammer()
    {
        if (shootTicks < 0 && (abs(shootTicks) % 10) == 0)
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 0.0, 0.0);
        }
        else
        {
            setBlitzMugenAnimationColor(fightEnemyEntity, 1.0, 1.0, 1.0);
        }
        shootTicks++;
        if (shootTicks < 0) return;

        if (getBlitzMugenAnimationAnimationNumber(fightEnemyEntity) == getEnemyBaseAnimationNo())
        {
            changeBlitzMugenAnimation(fightEnemyEntity, getEnemyBaseAnimationNo() + 3);
        }
        if (shotPattern <= 30)
        {
            // circle pattern
            if (shootTicks % 20 == 0)
            {
                double angleOffset = randfrom(0, M_PI);
                for (int i = 0; i < 10; i++)
                {
                    double anglePlus = i * ((2.0 * M_PI) / 10.0);
                    addAngledShot(angleOffset + anglePlus);
                }
            }
            if (shootTicks == 180)
            {
                startFightEnemyWait(240);
            }
        }
        else
        {
            // spin2win
            if (shootTicks % 3 == 0)
            {
                double angle = shootTicks * 0.1;
                double angleOffset = (shootTicks / 60.0) * 0.2;
                addAngledShot(angleOffset + angle);
            }
            if (shootTicks == 180)
            {
                startFightEnemyWait(180);
            }
        }
    }

    void updateFightEnemyMovement()
    {
        if (gGameScreenData.mLevel == 0)
        {
            updateFightEnemyMovementKasha();
        } 
        else if (gGameScreenData.mLevel == 1)
        {
            updateFightEnemyMovementBroni();
        }
        else
        {
            updateFightEnemyMovementYammer();
        }
    }

    void updateFightEnemyShoot()
    {
        if (gGameScreenData.mLevel == 0)
        {
            updateFightEnemyShootKasha();
        }
        else if (gGameScreenData.mLevel == 1)
        {
            updateFightEnemyShootBroni();
        }
        else
        {
            updateFightEnemyShootYammer();
        }
    }

    void updateFightEnemyWait()
    {
        if (gGameScreenData.mLevel == 0)
        {
            updateFightEnemyWaitKasha();
        }
        else if (gGameScreenData.mLevel == 1)
        {
            updateFightEnemyWaitBroni();
        }
        else
        {
            updateFightEnemyWaitYammer();
        }
    }

    // Bullets
    class Bullet
    {
    public:
        bool isToBeDeleted = false;
        int radius;
        int entityId;
        Vector2D direction;
    };
    std::map<int, Bullet> mBullets;

    void loadBullets() {}
    
    void updateBullets() {
        updateBulletMovement();
        updateBulletHit();
        updateBulletRemoval();
    }
    
    void updateBulletMovement()
    {
        for (auto& bp : mBullets)
        {
            auto& b = bp.second;
            addBlitzEntityPosition(b.entityId, b.direction);
            if (!checkPointInRectangle(GeoRectangle2D(-10, -10, 340, 260), getBlitzEntityPosition(b.entityId).xy()))
            {
                b.isToBeDeleted = true;
            }
        }
    }

    void updateBulletHit()
    {
        auto basePos = getBlitzEntityPosition(fightCharEntity);
        auto playerPos = basePos.xy() - Vector2DI(0, 12);
        for (auto& bp : mBullets)
        {
            auto& b = bp.second;
            auto bulletPos = getBlitzEntityPosition(b.entityId).xy();
            auto dist = vecLength(bulletPos - playerPos);
            if (dist < b.radius)
            {
                addPlayerDamage();
                b.isToBeDeleted = true;
            }
        }
    }

    void updateBulletRemoval() {
        for (auto& bp : mBullets)
        {
            auto& b = bp.second;
            if (b.isToBeDeleted)
            {
                removeBullet(b);
            }
        }

        auto it = mBullets.begin();
        while (it != mBullets.end())
        {
            if (it->second.isToBeDeleted) {
                it = mBullets.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void clearAllBullets()
    {
        for (auto& bp : mBullets)
        {
            auto& b = bp.second;
            b.isToBeDeleted = true;
        }
    }

    void addBullet(const Vector2D& pos, const Vector2D& dir, int animNo, int radius)
    {
        tryPlayMugenSoundAdvanced(&mSounds, 2, 4, sfxVolume);
        int bulletEntity = addBlitzEntity(pos.xyz(40));
        addBlitzMugenAnimationComponent(bulletEntity, &mSprites, &mAnimations, animNo);
        mBullets[bulletEntity] = Bullet{ false, radius, bulletEntity, dir };
    }

    void removeBullet(const Bullet& b)
    {
        assert(b.isToBeDeleted);
        removeBlitzEntity(b.entityId);
    }
};

EXPORT_SCREEN_CLASS(GameScreen);

void resetGame()
{
    for (int i = 0; i < 3; i++) gGameScreenData.shownNarration[i] = false;
    gGameScreenData.mLevel = 0;
    gGameScreenData.mGameTicks = 0;
}

std::string getSpeedRunString()
{
    int totalSeconds = gGameScreenData.mGameTicks / 60;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    int milliseconds = (gGameScreenData.mGameTicks % 60) * 1000 / 60;
    return std::to_string(minutes) + "m " + std::to_string(seconds) + "s " + std::to_string(milliseconds) + "ms.";
}