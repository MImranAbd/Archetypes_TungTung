#pragma once

#include <raylib.h>
#include "ui.h"

namespace PauseUi
{
    constexpr int bannerHeight = 150;

    Color PageBannerColor(PausePage page);
    const char* PageTitle(PausePage page);
    Color ItemCategoryColor(ItemCategory category);

    void DrawBar(float x, float y, float width, float height,
        float fraction, Color fill);
    void DrawFace(int index, float x, float y, float size);
    void DrawPartyList(const Party& party, int x, int y, int width, int selected,
        bool targeting = false, int targetIndex = 0, bool targetAll = false);
    void DrawPaperPanel(Rectangle panel);
    void DrawBanner(PausePage page, const Party& party);
    void DrawHint(const char* text);
    void DrawRootMenu(int menuIndex, const Party& party);

    void DrawItemPage(const Party& party, const Inventory& inventory,
        const PauseMenuState& menu);
    void DrawEquipPage(const Party& party, const Inventory& inventory,
        const PauseMenuState& menu);

    void DrawSkillsPage(const Party& party, int memberIndex);
    void DrawQuestsPage(const Party& party);
    void DrawMemberPage(const Party& party, int memberIndex);
    void DrawSystemsPage(const Party& party, int dungeonFloor);
}
