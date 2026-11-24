#include "ui.h"
#include "ui_battle_internal.h"
#include "ui_internal.h"
#include "theme.h"

#include <raylib.h>
#include <cmath>
#include <string>
#include <vector>

using namespace UiInternal;
using namespace BattleUi;
void UI::DrawBattle(Battle& battle)
{
    if (battle.GetPhase() == Battle::Phase::Victory)
    {
        DrawBattleVictory(battle);
        return;
    }

    if (battle.GetPhase() == Battle::Phase::Defeat)
    {
        DrawBattleDefeat();
        return;
    }

    DrawBackdrop();

    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    const Party& party = battle.GetParty();
    const Character& active = party.Active();
    const std::vector<Enemy>& enemies = battle.GetEnemies();
    const int activeIndex = party.GetActiveIndex();
    const bool playerTurn = (battle.GetPhase() == Battle::Phase::PlayerTurn);

    int livingAllies = 0;
    for (const Character& member : party.Members())
    {
        if (member.hp > 0) livingAllies++;
    }

    int livingEnemies = 0;
    for (const Enemy& enemy : enemies)
    {
        if (enemy.IsAlive()) livingEnemies++;
    }

    int selected = battle.GetSelectedTarget();
    if (selected < 0 || selected >= static_cast<int>(enemies.size()))
        selected = 0;

    int selectedAlly = battle.GetSelectedAlly();
    if (selectedAlly < 0 || selectedAlly >= static_cast<int>(party.Members().size()))
        selectedAlly = activeIndex;

    bool targetingAlly = false;
    const Battle::MenuState menuState = battle.GetMenuState();
    const Battle::PendingAction pendingAction = battle.GetPendingAction();

    if (menuState == Battle::MenuState::Target)
    {
        if (pendingAction == Battle::PendingAction::Item)
        {
            targetingAlly = true;
        }
        else if (pendingAction == Battle::PendingAction::Skill)
        {
            const int skillIndex = battle.GetSkillMenuIndex();

            if (skillIndex >= 0 && skillIndex < static_cast<int>(active.skills.size()) &&
                active.level >= active.skills[skillIndex]->GetLevelRequired())
            {
                targetingAlly = active.skills[skillIndex]->TargetsAlly();
            }
        }
    }

    const bool showTargetCursor = (menuState == Battle::MenuState::Target);

    Camera2D camera = {};
    camera.target = { 0.0f, 0.0f };
    camera.offset = { 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    const float bannerTimer = battle.GetActionBannerTimer();
    const bool cinematic = battle.IsActionCinematic();

    float actionAmount = 0.0f;
    bool actionActive = (bannerTimer > 0.0f);
    const bool widePartyAction = actionActive && battle.IsActionWideParty();
    Vector2 actionActorPos = { 0.0f, 0.0f };
    Vector2 actionTargetPos = { 0.0f, 0.0f };
    bool actionActorOnParty = false;
    bool actionTargetOnParty = false;
    bool hasActionActor = false;
    bool hasActionTarget = false;
    int actionActorIndex = -1;
    int actionTargetIndex = -1;

    if (actionActive)
    {
        float progress = 1.0f - bannerTimer / Battle::actionDuration;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        if (progress < 0.12f)
        {
            const float u = progress / 0.12f;
            actionAmount = u * u * (3.0f - 2.0f * u);
        }
        else if (progress < 0.86f)
        {
            actionAmount = 1.0f;
        }
        else
        {
            const float u = (1.0f - progress) / 0.14f;
            actionAmount = u * u * (3.0f - 2.0f * u);
        }

        const int lastParty = battle.GetLastActorParty();
        const int lastEnemy = battle.GetLastActorEnemy();

        Vector2 actorNormal = { 0.0f, 0.0f };
        Vector2 targetNormal = { 0.0f, 0.0f };

        if (lastParty >= 0 && lastParty < 4)
        {
            hasActionActor = true;
            actionActorOnParty = true;
            actionActorIndex = lastParty;
            actorNormal = PartyPos[lastParty];
        }
        else if (lastEnemy >= 0 && lastEnemy < 5)
        {
            hasActionActor = true;
            actionActorOnParty = false;
            actionActorIndex = lastEnemy;
            actorNormal = EnemyPos[lastEnemy];
        }

        if (battle.HasLastActionTarget())
        {
            const int target = battle.GetLastActionTarget();

            if (battle.IsLastActionTargetOnParty() && target >= 0 && target < 4)
            {
                hasActionTarget = true;
                actionTargetOnParty = true;
                actionTargetIndex = target;
                targetNormal = PartyPos[target];
            }
            else if (!battle.IsLastActionTargetOnParty() && target >= 0 && target < 5)
            {
                hasActionTarget = true;
                actionTargetOnParty = false;
                actionTargetIndex = target;
                targetNormal = EnemyPos[target];
            }
        }

        Vector2 actorDest = { width * 0.5f, height * 0.55f };
        Vector2 targetDest = { width * 0.5f, height * 0.55f };

        if (hasActionActor && hasActionTarget)
        {
            if (actionActorOnParty == actionTargetOnParty)
            {
                actorDest = { width * 0.42f, height * 0.55f };
                targetDest = { width * 0.58f, height * 0.55f };
            }
            else if (actionActorOnParty)
            {
                actorDest = { width * 0.38f, height * 0.55f };
                targetDest = { width * 0.62f, height * 0.55f };
            }
            else
            {
                actorDest = { width * 0.62f, height * 0.55f };
                targetDest = { width * 0.38f, height * 0.55f };
            }
        }

        actionActorPos = {
            actorNormal.x + (actorDest.x - actorNormal.x) * actionAmount,
            actorNormal.y + (actorDest.y - actorNormal.y) * actionAmount
        };

        actionTargetPos = {
            targetNormal.x + (targetDest.x - targetNormal.x) * actionAmount,
            targetNormal.y + (targetDest.y - targetNormal.y) * actionAmount
        };

        float focusX = width / 2.0f;
        float focusY = height * 0.55f;

        if (!widePartyAction)
        {
            if (hasActionActor && hasActionTarget)
            {
                focusX = (actionActorPos.x + actionTargetPos.x) / 2.0f;
                focusY = (actionActorPos.y + actionTargetPos.y) / 2.0f;
            }
            else if (hasActionActor)
            {
                focusX = actionActorPos.x;
                focusY = actionActorPos.y;
            }
            else if (hasActionTarget)
            {
                focusX = actionTargetPos.x;
                focusY = actionTargetPos.y;
            }
        }

        camera.zoom = 1.0f + 0.95f * actionAmount;
        camera.offset = { focusX - focusX * camera.zoom, focusY - focusY * camera.zoom };
    }

    Vector2 partyPositions[4];

    for (int i = 0; i < 4; i++)
    {
        Vector2 pos = PartyPos[i];

        if (widePartyAction)
        {
            const float groupX[4] = { 0.32f, 0.44f, 0.56f, 0.68f };
            const Vector2 dest = { width * groupX[i], height * 0.55f };

            pos = {
                PartyPos[i].x + (dest.x - PartyPos[i].x) * actionAmount,
                PartyPos[i].y + (dest.y - PartyPos[i].y) * actionAmount
            };
        }
        else if (actionActive)
        {
            if (hasActionActor && actionActorOnParty && i == actionActorIndex)
                pos = actionActorPos;
            else if (hasActionTarget && actionTargetOnParty && i == actionTargetIndex)
                pos = actionTargetPos;
        }

        partyPositions[i] = pos;
    }

    if (actionActive)
    {
        DrawRectangle(0, 120, width, height - 410,
            Fade(Color{ 8, 5, 5, 255 }, 0.86f * actionAmount));
    }

    BeginMode2D(camera);

    for (int x = 0; x < width; x += 16)
    {
        const float t1 = static_cast<float>(x) / width;
        const float t2 = static_cast<float>(x + 16) / width;
        const int y1 = static_cast<int>(height * 0.68f + std::sin(t1 * PI) * 24.0f);
        const int y2 = static_cast<int>(height * 0.68f + std::sin(t2 * PI) * 24.0f);

        DrawLine(x, y1, x + 16, y2, Fade(Theme::Ink, 0.35f));
    }

    const float buffAlpha = battle.GetBuffFxTimer() / 0.6f;
    Color buffColor = Theme::Red;

    if (buffAlpha > 0.0f)
    {
        const SkillKind fxKind = battle.GetBuffFxKind();

        if (fxKind == SkillKind::Heal ||
            fxKind == SkillKind::HealAll)
            buffColor = { 120, 210, 140, 255 };
        else if (fxKind == SkillKind::BuffDefense ||
            fxKind == SkillKind::BuffDefenseAll)
            buffColor = Theme::Green;
        else if (fxKind == SkillKind::BuffEvasion ||
            fxKind == SkillKind::BuffEvasionAll ||
            fxKind == SkillKind::DebuffAccuracy ||
            fxKind == SkillKind::DebuffAccuracyAll)
            buffColor = Theme::Purple;

        const int fxTarget = battle.GetBuffFxTarget();

        if (battle.IsBuffFxOnParty())
        {
            if (fxTarget >= 0 && fxTarget < 4)
            {
                if (party.Members()[fxTarget].hp > 0)
                {
                    Vector2 fxPos = partyPositions[fxTarget];

                    DrawEllipse(static_cast<int>(fxPos.x),
                        static_cast<int>(fxPos.y) + 58, 50.0f, 14.0f,
                        Fade(buffColor, 0.18f * buffAlpha));
                    DrawEllipseLines(static_cast<int>(fxPos.x),
                        static_cast<int>(fxPos.y) + 58, 50.0f, 14.0f,
                        Fade(buffColor, buffAlpha));
                }
            }
            else
            {
                for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
                {
                    if (party.Members()[i].hp <= 0) continue;

                    DrawEllipse(static_cast<int>(partyPositions[i].x),
                        static_cast<int>(partyPositions[i].y) + 58, 50.0f, 14.0f,
                        Fade(buffColor, 0.18f * buffAlpha));
                    DrawEllipseLines(static_cast<int>(partyPositions[i].x),
                        static_cast<int>(partyPositions[i].y) + 58, 50.0f, 14.0f,
                        Fade(buffColor, buffAlpha));
                }
            }
        }
        else if (fxTarget >= 0 && fxTarget < 5)
        {
            if (enemies[fxTarget].IsAlive())
            {
                Vector2 fxPos = EnemyPos[fxTarget];
                if (actionActive && !actionTargetOnParty && fxTarget == actionTargetIndex)
                    fxPos = actionTargetPos;

                const float y = fxPos.y + EnemySizes[fxTarget] / 2.0f + 8.0f;
                DrawEllipse(static_cast<int>(fxPos.x),
                    static_cast<int>(y), 48.0f, 12.0f, Fade(buffColor, 0.18f * buffAlpha));
                DrawEllipseLines(static_cast<int>(fxPos.x),
                    static_cast<int>(y), 48.0f, 12.0f, Fade(buffColor, buffAlpha));
            }
        }
        else
        {
            for (int i = 0; i < static_cast<int>(enemies.size()); i++)
            {
                if (!enemies[i].IsAlive()) continue;

                const float y = EnemyPos[i].y + EnemySizes[i] / 2.0f + 8.0f;
                DrawEllipse(static_cast<int>(EnemyPos[i].x),
                    static_cast<int>(y), 48.0f, 12.0f, Fade(buffColor, 0.18f * buffAlpha));
                DrawEllipseLines(static_cast<int>(EnemyPos[i].x),
                    static_cast<int>(y), 48.0f, 12.0f, Fade(buffColor, buffAlpha));
            }
        }
    }

    int attackingIndex = (battle.GetSlashTimer() > 0.0f)
        ? battle.GetSlashActor() : -1;

    if (attackingIndex < 0 && battle.IsActionAttackPose() &&
        battle.GetActionBannerTimer() > 0.0f)
        attackingIndex = battle.GetLastActorParty();

    const bool allyPairAction = actionActive &&
        battle.GetLastActorParty() >= 0 &&
        battle.HasLastActionTarget() &&
        battle.IsLastActionTargetOnParty();

    for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
    {
        if (allyPairAction && i != actionActorIndex && i != actionTargetIndex)
            continue;

        Vector2 pos = partyPositions[i];

        const int pointerIndex = (bannerTimer > 0.0f) ? -1 : activeIndex;

        Color actorTint = BLANK;

        if (buffAlpha > 0.0f && battle.IsBuffFxOnParty())
        {
            const int fxTarget = battle.GetBuffFxTarget();

            if (fxTarget < 0 || fxTarget == i)
                actorTint = Fade(buffColor, 0.45f * buffAlpha);
        }

        DrawPartyActor(i, pos, pointerIndex, attackingIndex, party, actorTint);
    }

    if (!allyPairAction && !widePartyAction)
    {
        for (int i = 0; i < static_cast<int>(enemies.size()); i++)
        {
            Vector2 pos = EnemyPos[i];

            if (actionActive)
            {
                if (hasActionActor && !actionActorOnParty && i == actionActorIndex)
                    pos = actionActorPos;
                else if (hasActionTarget && !actionTargetOnParty && i == actionTargetIndex)
                    pos = actionTargetPos;
            }

            const bool showEnemyCursor = showTargetCursor && !targetingAlly && i == selected;

            Color enemyTint = BLANK;

            if (buffAlpha > 0.0f && !battle.IsBuffFxOnParty())
            {
                const int fxTarget = battle.GetBuffFxTarget();

                if (fxTarget < 0 || fxTarget == i)
                    enemyTint = Fade(buffColor, 0.45f * buffAlpha);
            }

            const bool enemyActing = (bannerTimer > 0.0f &&
                battle.GetLastActorEnemy() == i);
            const float enemyScale = battle.IsElite()
                ? (i == 0 ? 2.75f : 0.65f)
                : 1.0f;

            DrawEnemyActor(i, pos, showEnemyCursor, enemies[i], enemyTint,
                enemyActing, enemyScale);
        }
    }

    const bool reviveTargeting = battle.IsReviveTargeting();

    if (showTargetCursor && targetingAlly && selectedAlly >= 0 && selectedAlly < 4 &&
        (reviveTargeting ? party.Members()[selectedAlly].hp <= 0
            : party.Members()[selectedAlly].hp > 0))
    {
        DrawTargetCaret(PartyPos[selectedAlly].x, PartyPos[selectedAlly].y - 104.0f,
            Theme::Gold);
    }

    const float flashAlpha = battle.GetHitFlashTimer() / Battle::hitFlashDuration;

    if (flashAlpha > 0.0f)
    {
        const int flashTarget = battle.GetHitFlashTarget();

        if (battle.IsHitFlashOnParty())
        {
            if (flashTarget >= 0 && flashTarget < 4)
            {
                Vector2 flashPos = partyPositions[flashTarget];

                DrawHitFlash(static_cast<int>(flashPos.x),
                    static_cast<int>(flashPos.y), 1.0f, flashAlpha);
            }
        }
        else if (flashTarget >= 0 && flashTarget < 5)
        {
            Vector2 flashPos = EnemyPos[flashTarget];
            if (actionActive && !actionTargetOnParty && flashTarget == actionTargetIndex)
                flashPos = actionTargetPos;

            DrawHitFlash(static_cast<int>(flashPos.x),
                static_cast<int>(flashPos.y), 0.85f, flashAlpha);
        }
    }

    for (const ActionText& text : battle.GetActionTexts())
    {
        if (text.target < 0) continue;

        Vector2 position = { 0.0f, 0.0f };

        if (text.onParty)
        {
            if (text.target >= static_cast<int>(party.Members().size())) continue;
            position = partyPositions[text.target];
        }
        else
        {
            if (text.target >= static_cast<int>(enemies.size())) continue;
            position = EnemyPos[text.target];

            if (actionActive && hasActionTarget && !actionTargetOnParty &&
                text.target == actionTargetIndex)
                position = actionTargetPos;
        }

        float fade = text.timer / Battle::supportTextDuration;
        if (fade < 0.0f) fade = 0.0f;
        if (fade > 1.0f) fade = 1.0f;

        const int rise = static_cast<int>((1.0f - fade) * 36.0f);
        const int x = static_cast<int>(position.x) + (text.onParty ? 28 : 38);
        const int y = static_cast<int>(position.y) -
            (text.onParty ? 118 : 100) - rise;

        DrawCentered(text.text.c_str(), x, y, 24,
            Fade(text.color, fade), true);
    }

    for (const DamagePopup& popup : battle.GetDamagePopups())
    {
        if (actionActive && actionAmount < 0.5f) continue;

        Vector2 position = { 0.0f, 0.0f };
        float sizeScale = 1.0f;

        if (popup.onParty)
        {
            if (popup.target < 0 || popup.target >= 4) continue;
            position = partyPositions[popup.target];
            sizeScale = 1.0f;
        }
        else
        {
            if (popup.target < 0 || popup.target >= 5) continue;
            position = EnemyPos[popup.target];
            if (actionActive && !actionTargetOnParty && popup.target == actionTargetIndex)
                position = actionTargetPos;
            sizeScale = 0.9f;
        }

        const float fade = popup.timer / Battle::damagePopupDuration;
        const int rise = static_cast<int>((1.0f - fade) * 55.0f);
        const int textSize = popup.crit ? 48 : 38;
        const Color color = popup.crit ? Theme::Red : WHITE;
        const int textX = static_cast<int>(position.x) + (popup.onParty ? 28 : 38);
        const int textY = static_cast<int>(position.y - 78.0f * sizeScale) - rise;

        DrawCentered(TextFormat("%d", popup.amount), textX, textY, textSize,
            Fade(color, fade), true);

        if (popup.weak)
        {
            DrawCentered("WEAK!", textX, textY - (popup.crit ? 46 : 24), 20,
                Fade(Theme::Gold, fade), true);
        }

        if (popup.crit)
        {
            DrawCentered("CRIT", textX, textY - 24, 18,
                Fade(Theme::Red, fade), true);
        }
    }

    EndMode2D();

    const float supportAlpha = battle.GetSupportFlashTimer() /
        Battle::supportFlashDuration;

    if (supportAlpha > 0.0f)
    {
        DrawRectangle(0, 0, width, height,
            Fade(battle.GetSupportFlashColor(), 0.10f * supportAlpha));
    }

    const float impactAlpha = battle.GetHitFlashTimer() / Battle::hitFlashDuration;

    if (actionActive && impactAlpha > 0.0f)
    {
        DrawRectangle(0, 0, width, height,
            Fade(Color{ 255, 235, 210, 255 }, 0.07f * impactAlpha));
    }

    DrawPageFrame();

    const int iconPoints = playerTurn ? battle.GetPlayerPoints() : battle.GetEnemyPoints();
    const int halfIcons = playerTurn ? battle.GetPlayerHalfIcons() : battle.GetEnemyHalfIcons();
    const int maxIcons = playerTurn
        ? livingAllies
        : livingEnemies * (battle.IsElite() ? 2 : 1);

    DrawTurnSquares(iconPoints, halfIcons, maxIcons, 40, 26,
        playerTurn ? Theme::Ink : Theme::Red);

    const int titleX = 40 + maxIcons * 63 + 18;

    UiDrawText("Press Turn", titleX, 12, 44, Theme::Ink, true);
    UiDrawText(TextFormat("Turn %d / %s", battle.GetTurnNumber(),
        playerTurn ? "Player" : "Enemy"), titleX, 62, 20, Theme::Muted);

    const Rectangle descriptionBox = { 610.0f, 14.0f, 750.0f, 104.0f };
    DrawRectangleRec(descriptionBox, Theme::Paper);
    DrawRectangleLinesEx(descriptionBox, 2.0f, Theme::Ink);
    DrawWrappedText(battle.GetMessage().c_str(), 628, 28, 714, 22, Theme::Ink);

    const char* battleTitle = battle.IsElite() ? "TREMENDOUS FOE" : "RANDOM BATTLE";
    UiDrawText(battleTitle, width - 40 - UiMeasureText(battleTitle, 44, true),
        18, 44, Theme::Ink, true);

    if (battle.IsEnemySmirk())
        UiDrawText("Enemy Smirk", 1420, 150, 20, Theme::Gold, true);
    else if (battle.IsEnemyAnxious())
        UiDrawText("Enemy Anxious", 1420, 150, 20, Theme::Purple, true);

    if (bannerTimer > 0.0f && cinematic)
    {
        const float bannerAlpha = (bannerTimer < 0.25f) ? (bannerTimer / 0.25f) : 1.0f;
        const char* banner = battle.GetActionBanner().c_str();
        const int bannerWidth = UiMeasureText(banner, 64, true);

        DrawLine(width / 2 - bannerWidth / 2 - 20, height - 300,
            width / 2 + bannerWidth / 2 + 20, height - 300, Fade(Theme::Gold, bannerAlpha));
        DrawCentered(banner, width / 2, height - 364, 64, Fade(WHITE, bannerAlpha), true);
    }

    int highlightedCommand = battle.GetCommandIndex();

    if (menuState == Battle::MenuState::Skill)
        highlightedCommand = static_cast<int>(Battle::Command::Skill);
    else if (menuState == Battle::MenuState::Item)
        highlightedCommand = static_cast<int>(Battle::Command::Item);
    else if (menuState == Battle::MenuState::Target)
    {
        if (pendingAction == Battle::PendingAction::Attack)
            highlightedCommand = static_cast<int>(Battle::Command::Attack);
        else if (pendingAction == Battle::PendingAction::Skill)
            highlightedCommand = static_cast<int>(Battle::Command::Skill);
        else if (pendingAction == Battle::PendingAction::Item)
            highlightedCommand = static_cast<int>(Battle::Command::Item);
    }

    for (int i = 0; i < Battle::commandCount; i++)
    {
        const int y = 800 + i * 36;
        const bool highlighted = playerTurn && (i == highlightedCommand);
        const Color color = highlighted ? Theme::Red : (playerTurn ? Theme::Ink : Theme::Muted);

        UiDrawText(CommandLabel(i), 40, y, 30, color, highlighted);

        if (highlighted)
        {
            DrawLineEx({ 194.0f, static_cast<float>(y) + 15.0f },
                { 208.0f, static_cast<float>(y) + 15.0f }, 2.0f, Theme::Gold);
            DrawLineEx({ 202.0f, static_cast<float>(y) + 9.0f },
                { 208.0f, static_cast<float>(y) + 15.0f }, 2.0f, Theme::Gold);
            DrawLineEx({ 202.0f, static_cast<float>(y) + 21.0f },
                { 208.0f, static_cast<float>(y) + 15.0f }, 2.0f, Theme::Gold);
        }
    }

    const Rectangle listPanel = { 220.0f, 790.0f, 470.0f, 250.0f };
    DrawRectangleRec(listPanel, Theme::Paper);
    DrawRectangleLinesEx(listPanel, 2.0f, Theme::Ink);

    const bool showSkillList = (menuState == Battle::MenuState::Skill) ||
        (menuState == Battle::MenuState::Target && pendingAction == Battle::PendingAction::Skill);
    const bool showItemList = (menuState == Battle::MenuState::Item) ||
        (menuState == Battle::MenuState::Target && pendingAction == Battle::PendingAction::Item);

    if (showSkillList)
    {
        UiDrawText("SKILLS", 238, 802, 18, Theme::Gold, true);

        std::vector<int> unlocked;

        for (int i = 0; i < static_cast<int>(active.skills.size()); i++)
        {
            if (active.level >= active.skills[i]->GetLevelRequired())
                unlocked.push_back(i);
        }

        const int visibleRows = 5;
        int selectedPos = 0;

        for (int i = 0; i < static_cast<int>(unlocked.size()); i++)
        {
            if (unlocked[i] == battle.GetSkillMenuIndex())
            {
                selectedPos = i;
                break;
            }
        }

        int start = 0;

        if (static_cast<int>(unlocked.size()) > visibleRows)
        {
            start = selectedPos - visibleRows / 2;

            if (start < 0) start = 0;
            if (start + visibleRows > static_cast<int>(unlocked.size()))
                start = static_cast<int>(unlocked.size()) - visibleRows;
        }

        for (int row = 0; row < visibleRows; row++)
        {
            const int pos = start + row;

            if (pos >= static_cast<int>(unlocked.size()))
                break;

            const int skillIndex = unlocked[pos];
            const Skill& skill = *active.skills[skillIndex];
            const int y = 840 + row * 38;
            const bool selectedSkill =
                (battle.GetSkillMenuIndex() == skillIndex);

            if (selectedSkill)
            {
                DrawRectangle(232, y - 7, 446, 34, Fade(Theme::Gold, 0.16f));
                DrawRectangleLines(232, y - 7, 446, 34, Theme::Gold);
            }

            DrawRectangleLines(244, y, 12, 12, Theme::Ink);
            UiDrawText(skill.GetName(), 268, y - 5, 22, Theme::Ink, selectedSkill);
            UiDrawText(TextFormat("LV %d", skill.GetLevelRequired()),
                590, y - 2, 14, Theme::Muted);
        }

        if (start > 0)
            UiDrawText("^", 666, 800, 16, Theme::Muted);

        if (start + visibleRows < static_cast<int>(unlocked.size()))
            UiDrawText("v", 666, 1020, 16, Theme::Muted);
    }
    else if (showItemList)
    {
        UiDrawText("ITEMS", 238, 802, 18, Theme::Gold, true);

        for (int i = 0; i < Battle::itemMenuCount; i++)
        {
            const ItemType type = Battle::ItemAtMenuIndex(i);
            const int y = 840 + i * 44;
            const bool selectedItem = (battle.GetItemMenuIndex() == i);

            if (selectedItem)
            {
                DrawRectangle(232, y - 7, 446, 38, Fade(Theme::Gold, 0.16f));
                DrawRectangleLines(232, y - 7, 446, 38, Theme::Gold);
            }

            DrawRectangleLines(244, y, 12, 12, Theme::Ink);
            UiDrawText(ItemName(type), 268, y - 5, 22, Theme::Ink, selectedItem);
            UiDrawText(TextFormat("x%d", battle.GetInventory().Count(type)),
                590, y - 2, 16, Theme::Muted);
        }
    }
    else
    {
        DrawCentered("SKILL / ITEM LIST", 455, 895, 22, Theme::Muted);
    }

    const Rectangle detailPanel = { 720.0f, 790.0f, 310.0f, 250.0f };
    DrawRectangleRec(detailPanel, Theme::Paper);
    DrawRectangleLinesEx(detailPanel, 2.0f, Theme::Ink);

    const char* detailName = "";
    const char* detailText = "";

    if (showSkillList)
    {
        const int index = battle.GetSkillMenuIndex();

        if (index >= 0 && index < static_cast<int>(active.skills.size()))
        {
            const Skill& skill = *active.skills[index];
            detailName = skill.GetName();
            detailText = skill.GetDescription();
        }
    }
    else if (showItemList)
    {
        const ItemType type = Battle::ItemAtMenuIndex(battle.GetItemMenuIndex());
        detailName = ItemName(type);
        detailText = ItemDescription(type);
    }
    else if (menuState == Battle::MenuState::Target && pendingAction == Battle::PendingAction::Attack)
    {
        detailName = CommandLabel(static_cast<int>(Battle::Command::Attack));
        detailText = CommandDescription(static_cast<int>(Battle::Command::Attack));
    }
    else
    {
        detailName = CommandLabel(battle.GetCommandIndex());
        detailText = CommandDescription(battle.GetCommandIndex());
    }

    DrawRectangleLines(736, 808, 16, 16, Theme::Ink);
    UiDrawText(detailName, 762, 804, 20, Theme::Ink, true);
    DrawLine(736, 836, 1014, 836, Fade(Theme::Ink, 0.25f));
    DrawWrappedText(detailText, 738, 850, 274, 17, Theme::Ink);

    if (menuState == Battle::MenuState::Target)
    {
        if (targetingAlly)
            DrawCentered(TextFormat("Target: %s", party.Members()[selectedAlly].name),
                875, 1010, 15, Theme::Gold);
        else
            DrawCentered(TextFormat("Target: E%d", selected + 1), 875, 1010, 15, Theme::Gold);
    }
    DrawPartyCards(battle, 1050, 790, 830);
}





