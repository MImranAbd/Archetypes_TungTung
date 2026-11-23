#include "ui.h"
#include "ui_pause_internal.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>

using namespace UiInternal;
using namespace PauseUi;

void UI::DrawPauseMenu(const Party& party, const Inventory& inventory,
    const PauseMenuState& menu, int dungeonFloor) const
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    DrawRectangle(0, 0, width, height, Fade(BLACK, 0.62f));

    int member = menu.memberIndex;

    if (member < 0 || member >= static_cast<int>(party.Members().size()))
        member = 0;

    if (menu.page == PausePage::Root)
    {
        DrawRootMenu(menu.menuIndex, party);
        return;
    }

    PauseMenuState state = menu;
    state.memberIndex = member;

    if (state.page == PausePage::Items)
        DrawItemPage(party, inventory, state);
    else if (state.page == PausePage::Equip)
        DrawEquipPage(party, inventory, state);
    else if (state.page == PausePage::Skills)
        DrawSkillsPage(party, member);
    else if (state.page == PausePage::Quests)
        DrawQuestsPage(party);
    else if (state.page == PausePage::Member)
        DrawMemberPage(party, member);
    else if (state.page == PausePage::Systems)
        DrawSystemsPage(party, dungeonFloor);
}

