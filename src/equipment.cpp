#include "equipment.h"
#include "engine/engine.h"



WeaponMelee generate_weapon_melee(int32_t level) {
    WeaponMelee w{};
    w.level = level;
    w.slot = EQUIPMENT_SLOTS::WEAPON;
    switch (engine::random(0, 4)) {
        case 0:
        default:
            w.name = "Longsword";
            w.damage = engine::random(-level, level + 1);
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                    w.special = WEAPON_MELEE_SPECIAL::LIFE_STEAL;
                    w.damage += (int32_t)(MOD_MELEE_LIFE_STEAL * level);
                    break;
                case 1:
                    w.special = WEAPON_MELEE_SPECIAL::MELEE_ARMOUR_SHRED;
                    w.damage += (int32_t)(MOD_MELEE_ARMOUR_SHRED * level);
                    break;
                case 2:
                    w.special = WEAPON_MELEE_SPECIAL::SPIN;
                    w.damage += (int32_t)(MOD_MELEE_SPIN * level);
                    break;
                default:
                    w.special = WEAPON_MELEE_SPECIAL::MELEE_NOTHING;
                    w.damage += (int32_t)(MOD_MELEE_NOTHING * level);
                    break;
            }
            break;
        case 1:
            w.name = "Battle Axe";
            w.damage = engine::random(-level, 1);
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                    w.special = WEAPON_MELEE_SPECIAL::LIFE_STEAL;
                    w.damage += (int32_t)(MOD_MELEE_LIFE_STEAL * level);
                    break;
                case 1:
                case 2:
                    w.special = WEAPON_MELEE_SPECIAL::SPIN;
                    w.damage += (int32_t)(MOD_MELEE_SPIN * level);
                    break;
                default:
                    w.special = WEAPON_MELEE_SPECIAL::MELEE_NOTHING;
                    w.damage += (int32_t)(MOD_MELEE_NOTHING * level);
                    break;
            }
            break;
        case 2:
            w.name = "War Hammer";
            w.damage = engine::random(-level * 2, 1);
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                    w.special = WEAPON_MELEE_SPECIAL::LIFE_STEAL;
                    w.damage += (int32_t)(MOD_MELEE_LIFE_STEAL * level);
                    break;
                case 1:
                case 2:
                    w.special = WEAPON_MELEE_SPECIAL::MELEE_ARMOUR_SHRED;
                    w.damage += (int32_t)(MOD_MELEE_ARMOUR_SHRED * level);
                    break;
                default:
                    w.special = WEAPON_MELEE_SPECIAL::MELEE_NOTHING;
                    w.damage += (int32_t)(MOD_MELEE_NOTHING * level);
                    break;
            }
            break;
        case 4:
            w.name = "Scythe";
            w.damage = engine::random(-level * 2, 1);
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                case 1:
                    w.special = WEAPON_MELEE_SPECIAL::LIFE_STEAL;
                    w.damage += (int32_t)(MOD_MELEE_LIFE_STEAL * level);
                    break;
                case 2:
                    w.special = WEAPON_MELEE_SPECIAL::SPIN;
                    w.damage += (int32_t)(MOD_MELEE_SPIN * level);
                    break;
                default:
                    w.special = WEAPON_MELEE_SPECIAL::MELEE_NOTHING;
                    w.damage += (int32_t)(MOD_MELEE_NOTHING * level);
                    break;
            }
    }
    return w;
}


WeaponRanged generate_weapon_ranged(int32_t level) {
    WeaponRanged w{};
    w.level = level;
    w.slot = EQUIPMENT_SLOTS::WEAPON;
    switch (engine::random(0, 2)) {
        case 0:
            w.name = "Crossbow";
            w.damage = engine::random(0, level + 1);
            w.range = 8 + level;
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                case 1:
                    w.special = WEAPON_RANGED_SPECIAL::RANGED_ARMOUR_SHRED;
                    w.damage += (int32_t)(MOD_RANGED_ARMOUR_SHRED * level);
                    break;
                case 2:
                    w.special = WEAPON_RANGED_SPECIAL::PIERCING;
                    w.damage += (int32_t)(MOD_RANGED_PIERCING * level);
                default:
                    w.special = WEAPON_RANGED_SPECIAL::RANGED_NOTHING;
                    w.damage += (int32_t)(MOD_RANGED_NOTHING * level);
                    break;
            }
            break;
        case 1:
            w.name = "Longbow";
            w.damage = engine::random(-level, level + 1);
            w.range = 10 + level;
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                    w.special = WEAPON_RANGED_SPECIAL::EXPLOSIVE;
                    w.damage += (int32_t)(MOD_RANGED_EXPLOSIVE * level);
                    break;
                case 1:
                case 2:
                    w.special = WEAPON_RANGED_SPECIAL::PIERCING;
                    w.damage += (int32_t)(MOD_RANGED_PIERCING * level);
                default:
                    w.special = WEAPON_RANGED_SPECIAL::RANGED_NOTHING;
                    w.damage += (int32_t)(MOD_RANGED_NOTHING * level);
                    break;
            }
            break;
        case 2:
            w.name = "Shortbow";
            w.damage = engine::random(-level, level + 1);
            w.range = 5 + level;
            switch (engine::random(0, std::max(3, 10 - level))) {
                case 0:
                case 1:
                    w.special = WEAPON_RANGED_SPECIAL::EXPLOSIVE;
                    w.damage += (int32_t)(MOD_RANGED_EXPLOSIVE * level);
                    break;
                case 2:
                    w.special = WEAPON_RANGED_SPECIAL::RANGED_ARMOUR_SHRED;
                    w.damage += (int32_t)(MOD_RANGED_ARMOUR_SHRED * level);
                default:
                    w.special = WEAPON_RANGED_SPECIAL::RANGED_NOTHING;
                    w.damage += (int32_t)(MOD_RANGED_NOTHING * level);
                    break;
            }
            break;
    }
    return w;
}


Armour generate_armour(int32_t level) {
    Armour a{};
    a.level = level;
    switch (engine::random(0, 3)) {
        case 0:
            a.slot = EQUIPMENT_SLOTS::HEAD;
            a.regen_speed = engine::random(1, 3);
            if (engine::random(0, level) > 2) {
                a.name = "Crown";
                a.special = ARMOUR_SPECIAL::WEALTHY;
                a.armour = level * 2 + engine::random(-level, 1);
                a.hp = level * 10 + engine::random(-level, 1);
                a.regen = level * 4 + engine::random(-level, 1);
            } else {
                a.name = "Helmet";
                a.special = ARMOUR_SPECIAL::ARMOUR_NOTHING;
                a.armour = level * 3 + engine::random(0, level + 1);
                a.hp = level * 15 + engine::random(0, level + 1);
                a.regen = level * 6 + engine::random(0, level + 1);
            }
            break;
        case 1:
            a.slot = EQUIPMENT_SLOTS::BODY;
            a.regen_speed = engine::random(2, 5);
            if (engine::random(0, level) > 2) {
                a.name = "Razorplate";
                a.special = ARMOUR_SPECIAL::THORNS;
                a.armour = level * 6 + engine::random(-level, 1);
                a.hp = level * 12 + engine::random(-level, 1);
                a.regen = level * 2 + engine::random(-level, 1);
            } else {
                a.name = "Plate Armour";
                a.special = ARMOUR_SPECIAL::ARMOUR_NOTHING;
                a.armour = level * 9 + engine::random(0, level + 1);
                a.hp = level * 18 + engine::random(0, level + 1);
                a.regen = level * 3 + engine::random(0, level + 1);
            }
            break;
        case 2:
            a.slot = EQUIPMENT_SLOTS::BOOTS;
            a.regen_speed = engine::random(1, 3);
            if (engine::random(0, level) > 2) {
                a.name = "Winged Boots";
                a.special = ARMOUR_SPECIAL::QUICK;
                a.armour = level * 2 + engine::random(-level, 1);
                a.hp = level * 8 + engine::random(-level, 1);
                a.regen = level * 2 + engine::random(-level, 1);
            } else {
                a.name = "Plated Boots";
                a.special = ARMOUR_SPECIAL::ARMOUR_NOTHING;
                a.armour = level * 3 + engine::random(0, level + 1);
                a.hp = level * 12 + engine::random(0, level + 1);
                a.regen = level * 3 + engine::random(0, level + 1);
            }
            break;
    }
    return a;
}
