#pragma once
// Generated from Production/Magic/spells.json; do not edit by hand.
#include "Magic/AskirMagicRules.h"
namespace AskirMagicRules {
inline constexpr Spell Catalog[] = {
    {"feuer_1","FeuerSto\u00df",Element::Fire,Effect::Projectile,14.0,2.5,21.0},
    {"feuer_2","FeuerSchild",Element::Fire,Effect::Shield,20.0,5.0,30.0},
    {"feuer_3","FeuerKlinge",Element::Fire,Effect::MeleeArc,26.0,7.5,39.0},
    {"feuer_4","FeuerNova",Element::Fire,Effect::Area,32.0,10.0,48.0},
    {"eis_1","EisSto\u00df",Element::Ice,Effect::Projectile,14.0,2.5,21.0},
    {"eis_2","EisSchild",Element::Ice,Effect::Shield,20.0,5.0,30.0},
    {"eis_3","EisKlinge",Element::Ice,Effect::MeleeArc,26.0,7.5,39.0},
    {"eis_4","EisNova",Element::Ice,Effect::Area,32.0,10.0,48.0},
    {"sturm_1","SturmSto\u00df",Element::Storm,Effect::Projectile,14.0,2.5,21.0},
    {"sturm_2","SturmSchild",Element::Storm,Effect::Shield,20.0,5.0,30.0},
    {"sturm_3","SturmKlinge",Element::Storm,Effect::MeleeArc,26.0,7.5,39.0},
    {"sturm_4","SturmNova",Element::Storm,Effect::Area,32.0,10.0,48.0},
    {"erde_1","ErdeSto\u00df",Element::Earth,Effect::Projectile,14.0,2.5,21.0},
    {"erde_2","ErdeSchild",Element::Earth,Effect::Shield,20.0,5.0,30.0},
    {"erde_3","ErdeKlinge",Element::Earth,Effect::MeleeArc,26.0,7.5,39.0},
    {"erde_4","ErdeNova",Element::Earth,Effect::Area,32.0,10.0,48.0},
    {"licht_1","LichtSto\u00df",Element::Light,Effect::Projectile,14.0,2.5,21.0},
    {"licht_2","LichtSchild",Element::Light,Effect::Shield,20.0,5.0,30.0},
    {"licht_3","LichtKlinge",Element::Light,Effect::MeleeArc,26.0,7.5,39.0},
    {"licht_4","LichtNova",Element::Light,Effect::Area,32.0,10.0,48.0},
    {"schatten_1","SchattenSto\u00df",Element::Shadow,Effect::Projectile,14.0,2.5,21.0},
    {"schatten_2","SchattenSchild",Element::Shadow,Effect::Shield,20.0,5.0,30.0},
    {"schatten_3","SchattenKlinge",Element::Shadow,Effect::MeleeArc,26.0,7.5,39.0},
    {"schatten_4","SchattenNova",Element::Shadow,Effect::Area,32.0,10.0,48.0},
};
inline constexpr int SpellCount = sizeof(Catalog)/sizeof(Catalog[0]);
static_assert(SpellCount==24);
}
