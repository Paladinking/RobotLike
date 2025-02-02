#include <cstdint>
#include <string>



enum EQUIPMENT_SLOTS {
    WEAPON,
    HEAD,
    BODY,
    BOOTS,
};

struct Equipment {
    EQUIPMENT_SLOTS slot;
    std::string name;

    // virtual void render(int32_t offset_x, int32_t offset_y) = 0;
};


enum WEAPON_MELEE_SPECIAL {
    MELEE_NOTHING,
    LIFE_STEAL,
    MELEE_ARMOUR_SHRED,
    SPIN,
};

static constexpr float MOD_MELEE_NOTHING = 3.5, MOD_MELEE_LIFE_STEAL = 2.0, MOD_MELEE_ARMOUR_SHRED = 2.5, MOD_MELEE_SPIN = 3.0;

struct WeaponMelee : public Equipment {
    int32_t damage, level;
    WEAPON_MELEE_SPECIAL special;
};


enum WEAPON_RANGED_SPECIAL {
    RANGED_NOTHING,
    RANGED_ARMOUR_SHRED,
    EXPLOSIVE,
    PIERCING,
};

static constexpr float MOD_RANGED_NOTHING = 2.5, MOD_RANGED_ARMOUR_SHRED = 1.5, MOD_RANGED_EXPLOSIVE = 2.0, MOD_RANGED_PIERCING = 2.0;

struct WeaponRanged : public Equipment {
    int32_t damage, range, level;
    WEAPON_RANGED_SPECIAL special;
};


enum ARMOUR_SPECIAL {
    ARMOUR_NOTHING,
    THORNS,
    WEALTHY,
    QUICK,
};

struct Armour : public Equipment {
    int32_t armour, hp, regen, regen_speed, level;
    ARMOUR_SPECIAL special;
};


WeaponMelee generate_weapon_melee(int32_t level);
WeaponRanged generate_weapon_ranged(int32_t level);
Armour generate_armour(int32_t level);