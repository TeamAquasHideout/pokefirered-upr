#include "global.h"
#include "gflib.h"
#include "decompress.h"
#include "graphics.h"
#include "item_menu_icons.h"
#include "constants/items.h"

enum {
    TAG_BAG = 100,
    TAG_SWAP_LINE,
    TAG_ITEM_ICON,
    TAG_ITEM_ICON_ALT,
};

#define NUM_SWAP_LINE_SPRITES 9

// Indexes for sItemMenuIconSpriteIds
enum {
    SPR_BAG,
    SPR_SWAP_LINE_START,
    SPR_ITEM_ICON = SPR_SWAP_LINE_START + NUM_SWAP_LINE_SPRITES,
    SPR_ITEM_ICON_ALT,
    SPR_COUNT
};

enum {
    ANIM_SWAP_LINE_START,
    ANIM_SWAP_LINE_MID,
    ANIM_SWAP_LINE_END,
};

enum {
    AFFINEANIM_BAG_IDLE,
    AFFINEANIM_BAG_SHAKE,
};

static EWRAM_DATA u8 sItemMenuIconSpriteIds[SPR_COUNT] = {0};
static EWRAM_DATA void *sItemIconTilesBuffer = NULL;
static EWRAM_DATA void *sItemIconTilesBufferPadded = NULL;

static void SpriteCB_BagVisualSwitchingPockets(struct Sprite *sprite);
static void SpriteCB_ShakeBagSprite(struct Sprite *sprite);

static const struct OamData sOamData_Bag = {
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
    .priority = 1,
    .paletteNum = 0
};

static const union AnimCmd sAnim_Bag_OpenPokeBallsPocket[] = {
    ANIMCMD_FRAME(   0, 5),
    ANIMCMD_FRAME(0x40, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_Bag_OpenItemsPocket[] = {
    ANIMCMD_FRAME(   0, 5),
    ANIMCMD_FRAME(0x80, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_Bag_OpenKeyItemsPocket[] = {
    ANIMCMD_FRAME(   0, 5),
    ANIMCMD_FRAME(0xc0, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_Bag[] = {
    [POCKET_ITEMS - 1]      = sAnim_Bag_OpenItemsPocket,
    [POCKET_KEY_ITEMS - 1]  = sAnim_Bag_OpenKeyItemsPocket,
    [POCKET_POKE_BALLS - 1] = sAnim_Bag_OpenPokeBallsPocket,
};

static const union AffineAnimCmd sAffineAnim_BagIdle[] = {
    AFFINEANIMCMD_FRAME(0x100, 0x100, 0, 0),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_BagShake[] = {
    AFFINEANIMCMD_FRAME(0, 0, -2, 2),
    AFFINEANIMCMD_FRAME(0, 0,  2, 4),
    AFFINEANIMCMD_FRAME(0, 0, -2, 4),
    AFFINEANIMCMD_FRAME(0, 0,  2, 2),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd *const sAffineAnimTable_Bag[] = {
    [AFFINEANIM_BAG_IDLE]  = sAffineAnim_BagIdle,
    [AFFINEANIM_BAG_SHAKE] = sAffineAnim_BagShake
};

const struct CompressedSpriteSheet gSpriteSheet_BagMale = {
    .data = gBagMale_Gfx,
    .size = 0x2000,
    .tag = TAG_BAG
};

const struct CompressedSpriteSheet gSpriteSheet_BagFemale = {
    .data = gBagFemale_Gfx,
    .size = 0x2000,
    .tag = TAG_BAG
};

const struct CompressedSpritePalette gSpritePalette_Bag = {
    .data = gBag_Pal,
    .tag = TAG_BAG
};

static const struct SpriteTemplate sSpriteTemplate_Bag = {
    .tileTag = TAG_BAG,
    .paletteTag = TAG_BAG,
    .oam = &sOamData_Bag,
    .anims = sAnims_Bag,
    .images = NULL,
    .affineAnims = sAffineAnimTable_Bag,
    .callback = SpriteCallbackDummy
};

static const struct OamData sOamData_SwapLine = {
    .affineMode = ST_OAM_AFFINE_OFF,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
    .paletteNum = 1
};

static const union AnimCmd sAnim_SwapLine_Start[] = {
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_SwapLine_Mid[] = {
    ANIMCMD_FRAME(4, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_SwapLine_End[] = {
    ANIMCMD_FRAME(0, 0, .hFlip = TRUE),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_SwapLine[] = {
    [ANIM_SWAP_LINE_START] = sAnim_SwapLine_Start,
    [ANIM_SWAP_LINE_MID]   = sAnim_SwapLine_Mid,
    [ANIM_SWAP_LINE_END]   = sAnim_SwapLine_End
};

const struct CompressedSpriteSheet gBagSwapSpriteSheet = {
    .data = gSwapLine_Gfx,
    .size = 0x100,
    .tag = TAG_SWAP_LINE
};

const struct CompressedSpritePalette gBagSwapSpritePalette = {
    .data = gSwapLine_Pal,
    .tag = TAG_SWAP_LINE
};

static const struct SpriteTemplate sSpriteTemplate_SwapLine = {
    .tileTag = TAG_SWAP_LINE,
    .paletteTag = TAG_SWAP_LINE,
    .oam = &sOamData_SwapLine,
    .anims = sAnims_SwapLine,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct OamData sOamData_ItemIcon = {
    .affineMode = ST_OAM_AFFINE_OFF,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
    .paletteNum = 2
};

static const union AnimCmd sAnim_ItemIcon[] = {
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_ItemIcon[] = {
    sAnim_ItemIcon
};

static const struct SpriteTemplate sSpriteTemplate_ItemIcon = {
    .tileTag = TAG_ITEM_ICON,
    .paletteTag = TAG_ITEM_ICON,
    .oam = &sOamData_ItemIcon,
    .anims = sAnims_ItemIcon,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const void *const sItemIconTable[ITEMS_COUNT + 1][2] =
{
    [ITEM_NONE] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_MASTER_BALL] = {gFile_graphics_items_icons_master_ball_sheet, gFile_graphics_items_icon_palettes_master_ball_palette},
    [ITEM_ULTRA_BALL] = {gFile_graphics_items_icons_ultra_ball_sheet, gFile_graphics_items_icon_palettes_ultra_ball_palette},
    [ITEM_GREAT_BALL] = {gFile_graphics_items_icons_great_ball_sheet, gFile_graphics_items_icon_palettes_great_ball_palette},
    [ITEM_POKE_BALL] = {gFile_graphics_items_icons_poke_ball_sheet, gFile_graphics_items_icon_palettes_poke_ball_palette},
    [ITEM_SAFARI_BALL] = {gFile_graphics_items_icons_safari_ball_sheet, gFile_graphics_items_icon_palettes_safari_ball_palette},
    [ITEM_NET_BALL] = {gFile_graphics_items_icons_net_ball_sheet, gFile_graphics_items_icon_palettes_net_ball_palette},
    [ITEM_DIVE_BALL] = {gFile_graphics_items_icons_dive_ball_sheet, gFile_graphics_items_icon_palettes_dive_ball_palette},
    [ITEM_NEST_BALL] = {gFile_graphics_items_icons_nest_ball_sheet, gFile_graphics_items_icon_palettes_nest_ball_palette},
    [ITEM_REPEAT_BALL] = {gFile_graphics_items_icons_repeat_ball_sheet, gFile_graphics_items_icon_palettes_repeat_ball_palette},
    [ITEM_TIMER_BALL] = {gFile_graphics_items_icons_timer_ball_sheet, gFile_graphics_items_icon_palettes_repeat_ball_palette},
    [ITEM_LUXURY_BALL] = {gFile_graphics_items_icons_luxury_ball_sheet, gFile_graphics_items_icon_palettes_luxury_ball_palette},
    [ITEM_PREMIER_BALL] = {gFile_graphics_items_icons_premier_ball_sheet, gFile_graphics_items_icon_palettes_luxury_ball_palette},
    [ITEM_POTION] = {gFile_graphics_items_icons_potion_sheet, gFile_graphics_items_icon_palettes_potion_palette},
    [ITEM_ANTIDOTE] = {gFile_graphics_items_icons_antidote_sheet, gFile_graphics_items_icon_palettes_antidote_palette},
    [ITEM_BURN_HEAL] = {gFile_graphics_items_icons_status_heal_sheet, gFile_graphics_items_icon_palettes_burn_heal_palette},
    [ITEM_ICE_HEAL] = {gFile_graphics_items_icons_status_heal_sheet, gFile_graphics_items_icon_palettes_ice_heal_palette},
    [ITEM_AWAKENING] = {gFile_graphics_items_icons_status_heal_sheet, gFile_graphics_items_icon_palettes_awakening_palette},
    [ITEM_PARALYZE_HEAL] = {gFile_graphics_items_icons_status_heal_sheet, gFile_graphics_items_icon_palettes_paralyze_heal_palette},
    [ITEM_FULL_RESTORE] = {gFile_graphics_items_icons_large_potion_sheet, gFile_graphics_items_icon_palettes_full_restore_palette},
    [ITEM_MAX_POTION] = {gFile_graphics_items_icons_large_potion_sheet, gFile_graphics_items_icon_palettes_max_potion_palette},
    [ITEM_HYPER_POTION] = {gFile_graphics_items_icons_potion_sheet, gFile_graphics_items_icon_palettes_hyper_potion_palette},
    [ITEM_SUPER_POTION] = {gFile_graphics_items_icons_potion_sheet, gFile_graphics_items_icon_palettes_super_potion_palette},
    [ITEM_FULL_HEAL] = {gFile_graphics_items_icons_full_heal_sheet, gFile_graphics_items_icon_palettes_full_heal_palette},
    [ITEM_REVIVE] = {gFile_graphics_items_icons_revive_sheet, gFile_graphics_items_icon_palettes_revive_palette},
    [ITEM_MAX_REVIVE] = {gFile_graphics_items_icons_max_revive_sheet, gFile_graphics_items_icon_palettes_revive_palette},
    [ITEM_FRESH_WATER] = {gFile_graphics_items_icons_fresh_water_sheet, gFile_graphics_items_icon_palettes_fresh_water_palette},
    [ITEM_SODA_POP] = {gFile_graphics_items_icons_soda_pop_sheet, gFile_graphics_items_icon_palettes_soda_pop_palette},
    [ITEM_LEMONADE] = {gFile_graphics_items_icons_lemonade_sheet, gFile_graphics_items_icon_palettes_lemonade_palette},
    [ITEM_MOOMOO_MILK] = {gFile_graphics_items_icons_moomoo_milk_sheet, gFile_graphics_items_icon_palettes_moomoo_milk_palette},
    [ITEM_ENERGY_POWDER] = {gFile_graphics_items_icons_powder_sheet, gFile_graphics_items_icon_palettes_energy_powder_palette},
    [ITEM_ENERGY_ROOT] = {gFile_graphics_items_icons_energy_root_sheet, gFile_graphics_items_icon_palettes_energy_root_palette},
    [ITEM_HEAL_POWDER] = {gFile_graphics_items_icons_powder_sheet, gFile_graphics_items_icon_palettes_heal_powder_palette},
    [ITEM_REVIVAL_HERB] = {gFile_graphics_items_icons_revival_herb_sheet, gFile_graphics_items_icon_palettes_revival_herb_palette},
    [ITEM_ETHER] = {gFile_graphics_items_icons_ether_sheet, gFile_graphics_items_icon_palettes_ether_palette},
    [ITEM_MAX_ETHER] = {gFile_graphics_items_icons_ether_sheet, gFile_graphics_items_icon_palettes_max_ether_palette},
    [ITEM_ELIXIR] = {gFile_graphics_items_icons_ether_sheet, gFile_graphics_items_icon_palettes_elixir_palette},
    [ITEM_MAX_ELIXIR] = {gFile_graphics_items_icons_ether_sheet, gFile_graphics_items_icon_palettes_max_elixir_palette},
    [ITEM_LAVA_COOKIE] = {gFile_graphics_items_icons_lava_cookie_sheet, gFile_graphics_items_icon_palettes_lava_cookie_and_letter_palette},
    [ITEM_BLUE_FLUTE] = {gFile_graphics_items_icons_flute_sheet, gFile_graphics_items_icon_palettes_blue_flute_palette},
    [ITEM_YELLOW_FLUTE] = {gFile_graphics_items_icons_flute_sheet, gFile_graphics_items_icon_palettes_yellow_flute_palette},
    [ITEM_RED_FLUTE] = {gFile_graphics_items_icons_flute_sheet, gFile_graphics_items_icon_palettes_red_flute_palette},
    [ITEM_BLACK_FLUTE] = {gFile_graphics_items_icons_flute_sheet, gFile_graphics_items_icon_palettes_black_flute_palette},
    [ITEM_WHITE_FLUTE] = {gFile_graphics_items_icons_flute_sheet, gFile_graphics_items_icon_palettes_white_flute_palette},
    [ITEM_BERRY_JUICE] = {gFile_graphics_items_icons_berry_juice_sheet, gFile_graphics_items_icon_palettes_berry_juice_palette},
    [ITEM_SACRED_ASH] = {gFile_graphics_items_icons_sacred_ash_sheet, gFile_graphics_items_icon_palettes_sacred_ash_palette},
    [ITEM_SHOAL_SALT] = {gFile_graphics_items_icons_powder_sheet, gFile_graphics_items_icon_palettes_shoal_salt_palette},
    [ITEM_SHOAL_SHELL] = {gFile_graphics_items_icons_shoal_shell_sheet, gFile_graphics_items_icon_palettes_shell_palette},
    [ITEM_RED_SHARD] = {gFile_graphics_items_icons_shard_sheet, gFile_graphics_items_icon_palettes_red_shard_palette},
    [ITEM_BLUE_SHARD] = {gFile_graphics_items_icons_shard_sheet, gFile_graphics_items_icon_palettes_blue_shard_palette},
    [ITEM_YELLOW_SHARD] = {gFile_graphics_items_icons_shard_sheet, gFile_graphics_items_icon_palettes_yellow_shard_palette},
    [ITEM_GREEN_SHARD] = {gFile_graphics_items_icons_shard_sheet, gFile_graphics_items_icon_palettes_green_shard_palette},
    [ITEM_034] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_035] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_036] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_037] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_038] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_039] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_03A] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_03B] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_03C] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_03D] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_03E] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_HP_UP] = {gFile_graphics_items_icons_hp_up_sheet, gFile_graphics_items_icon_palettes_hp_up_palette},
    [ITEM_PROTEIN] = {gFile_graphics_items_icons_vitamin_sheet, gFile_graphics_items_icon_palettes_protein_palette},
    [ITEM_IRON] = {gFile_graphics_items_icons_vitamin_sheet, gFile_graphics_items_icon_palettes_iron_palette},
    [ITEM_CARBOS] = {gFile_graphics_items_icons_vitamin_sheet, gFile_graphics_items_icon_palettes_carbos_palette},
    [ITEM_CALCIUM] = {gFile_graphics_items_icons_vitamin_sheet, gFile_graphics_items_icon_palettes_calcium_palette},
    [ITEM_RARE_CANDY] = {gFile_graphics_items_icons_rare_candy_sheet, gFile_graphics_items_icon_palettes_rare_candy_palette},
    [ITEM_PP_UP] = {gFile_graphics_items_icons_pp_up_sheet, gFile_graphics_items_icon_palettes_pp_up_palette},
    [ITEM_ZINC] = {gFile_graphics_items_icons_vitamin_sheet, gFile_graphics_items_icon_palettes_zinc_palette},
    [ITEM_PP_MAX] = {gFile_graphics_items_icons_pp_max_sheet, gFile_graphics_items_icon_palettes_pp_max_palette},
    [ITEM_048] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_GUARD_SPEC] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_guard_spec_palette},
    [ITEM_DIRE_HIT] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_dire_hit_palette},
    [ITEM_X_ATTACK] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_x_attack_palette},
    [ITEM_X_DEFEND] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_x_defend_palette},
    [ITEM_X_SPEED] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_x_speed_palette},
    [ITEM_X_ACCURACY] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_x_accuracy_palette},
    [ITEM_X_SPECIAL] = {gFile_graphics_items_icons_battle_stat_item_sheet, gFile_graphics_items_icon_palettes_x_special_palette},
    [ITEM_POKE_DOLL] = {gFile_graphics_items_icons_poke_doll_sheet, gFile_graphics_items_icon_palettes_poke_doll_palette},
    [ITEM_FLUFFY_TAIL] = {gFile_graphics_items_icons_fluffy_tail_sheet, gFile_graphics_items_icon_palettes_fluffy_tail_palette},
    [ITEM_052] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_SUPER_REPEL] = {gFile_graphics_items_icons_repel_sheet, gFile_graphics_items_icon_palettes_super_repel_palette},
    [ITEM_MAX_REPEL] = {gFile_graphics_items_icons_repel_sheet, gFile_graphics_items_icon_palettes_max_repel_palette},
    [ITEM_ESCAPE_ROPE] = {gFile_graphics_items_icons_escape_rope_sheet, gFile_graphics_items_icon_palettes_escape_rope_palette},
    [ITEM_REPEL] = {gFile_graphics_items_icons_repel_sheet, gFile_graphics_items_icon_palettes_repel_palette},
    [ITEM_057] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_058] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_059] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_05A] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_05B] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_05C] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_SUN_STONE] = {gFile_graphics_items_icons_sun_stone_sheet, gFile_graphics_items_icon_palettes_sun_stone_palette},
    [ITEM_MOON_STONE] = {gFile_graphics_items_icons_moon_stone_sheet, gFile_graphics_items_icon_palettes_moon_stone_palette},
    [ITEM_FIRE_STONE] = {gFile_graphics_items_icons_fire_stone_sheet, gFile_graphics_items_icon_palettes_fire_stone_palette},
    [ITEM_THUNDER_STONE] = {gFile_graphics_items_icons_thunder_stone_sheet, gFile_graphics_items_icon_palettes_thunder_stone_palette},
    [ITEM_WATER_STONE] = {gFile_graphics_items_icons_water_stone_sheet, gFile_graphics_items_icon_palettes_water_stone_palette},
    [ITEM_LEAF_STONE] = {gFile_graphics_items_icons_leaf_stone_sheet, gFile_graphics_items_icon_palettes_leaf_stone_palette},
    [ITEM_063] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_064] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_065] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_066] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_TINY_MUSHROOM] = {gFile_graphics_items_icons_tiny_mushroom_sheet, gFile_graphics_items_icon_palettes_mushroom_palette},
    [ITEM_BIG_MUSHROOM] = {gFile_graphics_items_icons_big_mushroom_sheet, gFile_graphics_items_icon_palettes_mushroom_palette},
    [ITEM_069] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_PEARL] = {gFile_graphics_items_icons_pearl_sheet, gFile_graphics_items_icon_palettes_pearl_palette},
    [ITEM_BIG_PEARL] = {gFile_graphics_items_icons_big_pearl_sheet, gFile_graphics_items_icon_palettes_pearl_palette},
    [ITEM_STARDUST] = {gFile_graphics_items_icons_stardust_sheet, gFile_graphics_items_icon_palettes_star_palette},
    [ITEM_STAR_PIECE] = {gFile_graphics_items_icons_star_piece_sheet, gFile_graphics_items_icon_palettes_star_palette},
    [ITEM_NUGGET] = {gFile_graphics_items_icons_nugget_sheet, gFile_graphics_items_icon_palettes_nugget_palette},
    [ITEM_HEART_SCALE] = {gFile_graphics_items_icons_heart_scale_sheet, gFile_graphics_items_icon_palettes_heart_scale_palette},
    [ITEM_070] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_071] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_072] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_073] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_074] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_075] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_076] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_077] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_078] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_ORANGE_MAIL] = {gFile_graphics_items_icons_orange_mail_sheet, gFile_graphics_items_icon_palettes_orange_mail_palette},
    [ITEM_HARBOR_MAIL] = {gFile_graphics_items_icons_harbor_mail_sheet, gFile_graphics_items_icon_palettes_harbor_mail_palette},
    [ITEM_GLITTER_MAIL] = {gFile_graphics_items_icons_glitter_mail_sheet, gFile_graphics_items_icon_palettes_glitter_mail_palette},
    [ITEM_MECH_MAIL] = {gFile_graphics_items_icons_mech_mail_sheet, gFile_graphics_items_icon_palettes_mech_mail_palette},
    [ITEM_WOOD_MAIL] = {gFile_graphics_items_icons_wood_mail_sheet, gFile_graphics_items_icon_palettes_wood_mail_palette},
    [ITEM_WAVE_MAIL] = {gFile_graphics_items_icons_wave_mail_sheet, gFile_graphics_items_icon_palettes_wave_mail_palette},
    [ITEM_BEAD_MAIL] = {gFile_graphics_items_icons_bead_mail_sheet, gFile_graphics_items_icon_palettes_bead_mail_palette},
    [ITEM_SHADOW_MAIL] = {gFile_graphics_items_icons_shadow_mail_sheet, gFile_graphics_items_icon_palettes_shadow_mail_palette},
    [ITEM_TROPIC_MAIL] = {gFile_graphics_items_icons_tropic_mail_sheet, gFile_graphics_items_icon_palettes_tropic_mail_palette},
    [ITEM_DREAM_MAIL] = {gFile_graphics_items_icons_dream_mail_sheet, gFile_graphics_items_icon_palettes_dream_mail_palette},
    [ITEM_FAB_MAIL] = {gFile_graphics_items_icons_fab_mail_sheet, gFile_graphics_items_icon_palettes_fab_mail_palette},
    [ITEM_RETRO_MAIL] = {gFile_graphics_items_icons_retro_mail_sheet, gFile_graphics_items_icon_palettes_retro_mail_palette},
    [ITEM_CHERI_BERRY] = {gFile_graphics_items_icons_cheri_berry_sheet, gFile_graphics_items_icon_palettes_cheri_berry_palette},
    [ITEM_CHESTO_BERRY] = {gFile_graphics_items_icons_chesto_berry_sheet, gFile_graphics_items_icon_palettes_chesto_berry_palette},
    [ITEM_PECHA_BERRY] = {gFile_graphics_items_icons_pecha_berry_sheet, gFile_graphics_items_icon_palettes_pecha_berry_palette},
    [ITEM_RAWST_BERRY] = {gFile_graphics_items_icons_rawst_berry_sheet, gFile_graphics_items_icon_palettes_rawst_berry_palette},
    [ITEM_ASPEAR_BERRY] = {gFile_graphics_items_icons_aspear_berry_sheet, gFile_graphics_items_icon_palettes_aspear_berry_palette},
    [ITEM_LEPPA_BERRY] = {gFile_graphics_items_icons_leppa_berry_sheet, gFile_graphics_items_icon_palettes_leppa_berry_palette},
    [ITEM_ORAN_BERRY] = {gFile_graphics_items_icons_oran_berry_sheet, gFile_graphics_items_icon_palettes_oran_berry_palette},
    [ITEM_PERSIM_BERRY] = {gFile_graphics_items_icons_persim_berry_sheet, gFile_graphics_items_icon_palettes_persim_berry_palette},
    [ITEM_LUM_BERRY] = {gFile_graphics_items_icons_lum_berry_sheet, gFile_graphics_items_icon_palettes_lum_berry_palette},
    [ITEM_SITRUS_BERRY] = {gFile_graphics_items_icons_sitrus_berry_sheet, gFile_graphics_items_icon_palettes_sitrus_berry_palette},
    [ITEM_FIGY_BERRY] = {gFile_graphics_items_icons_figy_berry_sheet, gFile_graphics_items_icon_palettes_figy_berry_palette},
    [ITEM_WIKI_BERRY] = {gFile_graphics_items_icons_wiki_berry_sheet, gFile_graphics_items_icon_palettes_wiki_berry_palette},
    [ITEM_MAGO_BERRY] = {gFile_graphics_items_icons_mago_berry_sheet, gFile_graphics_items_icon_palettes_mago_berry_palette},
    [ITEM_AGUAV_BERRY] = {gFile_graphics_items_icons_aguav_berry_sheet, gFile_graphics_items_icon_palettes_aguav_berry_palette},
    [ITEM_IAPAPA_BERRY] = {gFile_graphics_items_icons_iapapa_berry_sheet, gFile_graphics_items_icon_palettes_iapapa_berry_palette},
    [ITEM_RAZZ_BERRY] = {gFile_graphics_items_icons_razz_berry_sheet, gFile_graphics_items_icon_palettes_razz_berry_palette},
    [ITEM_BLUK_BERRY] = {gFile_graphics_items_icons_bluk_berry_sheet, gFile_graphics_items_icon_palettes_bluk_berry_palette},
    [ITEM_NANAB_BERRY] = {gFile_graphics_items_icons_nanab_berry_sheet, gFile_graphics_items_icon_palettes_nanab_berry_palette},
    [ITEM_WEPEAR_BERRY] = {gFile_graphics_items_icons_wepear_berry_sheet, gFile_graphics_items_icon_palettes_wepear_berry_palette},
    [ITEM_PINAP_BERRY] = {gFile_graphics_items_icons_pinap_berry_sheet, gFile_graphics_items_icon_palettes_pinap_berry_palette},
    [ITEM_POMEG_BERRY] = {gFile_graphics_items_icons_pomeg_berry_sheet, gFile_graphics_items_icon_palettes_pomeg_berry_palette},
    [ITEM_KELPSY_BERRY] = {gFile_graphics_items_icons_kelpsy_berry_sheet, gFile_graphics_items_icon_palettes_kelpsy_berry_palette},
    [ITEM_QUALOT_BERRY] = {gFile_graphics_items_icons_qualot_berry_sheet, gFile_graphics_items_icon_palettes_qualot_berry_palette},
    [ITEM_HONDEW_BERRY] = {gFile_graphics_items_icons_hondew_berry_sheet, gFile_graphics_items_icon_palettes_hondew_berry_palette},
    [ITEM_GREPA_BERRY] = {gFile_graphics_items_icons_grepa_berry_sheet, gFile_graphics_items_icon_palettes_grepa_berry_palette},
    [ITEM_TAMATO_BERRY] = {gFile_graphics_items_icons_tamato_berry_sheet, gFile_graphics_items_icon_palettes_tamato_berry_palette},
    [ITEM_CORNN_BERRY] = {gFile_graphics_items_icons_cornn_berry_sheet, gFile_graphics_items_icon_palettes_cornn_berry_palette},
    [ITEM_MAGOST_BERRY] = {gFile_graphics_items_icons_magost_berry_sheet, gFile_graphics_items_icon_palettes_magost_berry_palette},
    [ITEM_RABUTA_BERRY] = {gFile_graphics_items_icons_rabuta_berry_sheet, gFile_graphics_items_icon_palettes_rabuta_berry_palette},
    [ITEM_NOMEL_BERRY] = {gFile_graphics_items_icons_nomel_berry_sheet, gFile_graphics_items_icon_palettes_nomel_berry_palette},
    [ITEM_SPELON_BERRY] = {gFile_graphics_items_icons_spelon_berry_sheet, gFile_graphics_items_icon_palettes_spelon_berry_palette},
    [ITEM_PAMTRE_BERRY] = {gFile_graphics_items_icons_pamtre_berry_sheet, gFile_graphics_items_icon_palettes_pamtre_berry_palette},
    [ITEM_WATMEL_BERRY] = {gFile_graphics_items_icons_watmel_berry_sheet, gFile_graphics_items_icon_palettes_watmel_berry_palette},
    [ITEM_DURIN_BERRY] = {gFile_graphics_items_icons_durin_berry_sheet, gFile_graphics_items_icon_palettes_durin_berry_palette},
    [ITEM_BELUE_BERRY] = {gFile_graphics_items_icons_belue_berry_sheet, gFile_graphics_items_icon_palettes_belue_berry_palette},
    [ITEM_LIECHI_BERRY] = {gFile_graphics_items_icons_liechi_berry_sheet, gFile_graphics_items_icon_palettes_liechi_berry_palette},
    [ITEM_GANLON_BERRY] = {gFile_graphics_items_icons_ganlon_berry_sheet, gFile_graphics_items_icon_palettes_ganlon_berry_palette},
    [ITEM_SALAC_BERRY] = {gFile_graphics_items_icons_salac_berry_sheet, gFile_graphics_items_icon_palettes_salac_berry_palette},
    [ITEM_PETAYA_BERRY] = {gFile_graphics_items_icons_petaya_berry_sheet, gFile_graphics_items_icon_palettes_petaya_berry_palette},
    [ITEM_APICOT_BERRY] = {gFile_graphics_items_icons_apicot_berry_sheet, gFile_graphics_items_icon_palettes_apicot_berry_palette},
    [ITEM_LANSAT_BERRY] = {gFile_graphics_items_icons_lansat_berry_sheet, gFile_graphics_items_icon_palettes_lansat_berry_palette},
    [ITEM_STARF_BERRY] = {gFile_graphics_items_icons_starf_berry_sheet, gFile_graphics_items_icon_palettes_starf_berry_palette},
    [ITEM_ENIGMA_BERRY] = {gFile_graphics_items_icons_enigma_berry_sheet, gFile_graphics_items_icon_palettes_enigma_berry_palette},
    [ITEM_0B0] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0B1] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0B2] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_BRIGHT_POWDER] = {gFile_graphics_items_icons_bright_powder_sheet, gFile_graphics_items_icon_palettes_bright_powder_palette},
    [ITEM_WHITE_HERB] = {gFile_graphics_items_icons_in_battle_herb_sheet, gFile_graphics_items_icon_palettes_white_herb_palette},
    [ITEM_MACHO_BRACE] = {gFile_graphics_items_icons_macho_brace_sheet, gFile_graphics_items_icon_palettes_macho_brace_palette},
    [ITEM_EXP_SHARE] = {gFile_graphics_items_icons_exp_share_sheet, gFile_graphics_items_icon_palettes_exp_share_palette},
    [ITEM_QUICK_CLAW] = {gFile_graphics_items_icons_quick_claw_sheet, gFile_graphics_items_icon_palettes_quick_claw_palette},
    [ITEM_SOOTHE_BELL] = {gFile_graphics_items_icons_soothe_bell_sheet, gFile_graphics_items_icon_palettes_soothe_bell_palette},
    [ITEM_MENTAL_HERB] = {gFile_graphics_items_icons_in_battle_herb_sheet, gFile_graphics_items_icon_palettes_mental_herb_palette},
    [ITEM_CHOICE_BAND] = {gFile_graphics_items_icons_choice_band_sheet, gFile_graphics_items_icon_palettes_choice_band_palette},
    [ITEM_KINGS_ROCK] = {gFile_graphics_items_icons_kings_rock_sheet, gFile_graphics_items_icon_palettes_kings_rock_palette},
    [ITEM_SILVER_POWDER] = {gFile_graphics_items_icons_silver_powder_sheet, gFile_graphics_items_icon_palettes_silver_powder_palette},
    [ITEM_AMULET_COIN] = {gFile_graphics_items_icons_amulet_coin_sheet, gFile_graphics_items_icon_palettes_amulet_coin_palette},
    [ITEM_CLEANSE_TAG] = {gFile_graphics_items_icons_cleanse_tag_sheet, gFile_graphics_items_icon_palettes_cleanse_tag_palette},
    [ITEM_SOUL_DEW] = {gFile_graphics_items_icons_soul_dew_sheet, gFile_graphics_items_icon_palettes_soul_dew_palette},
    [ITEM_DEEP_SEA_TOOTH] = {gFile_graphics_items_icons_deep_sea_tooth_sheet, gFile_graphics_items_icon_palettes_deep_sea_tooth_palette},
    [ITEM_DEEP_SEA_SCALE] = {gFile_graphics_items_icons_deep_sea_scale_sheet, gFile_graphics_items_icon_palettes_deep_sea_scale_palette},
    [ITEM_SMOKE_BALL] = {gFile_graphics_items_icons_smoke_ball_sheet, gFile_graphics_items_icon_palettes_smoke_ball_palette},
    [ITEM_EVERSTONE] = {gFile_graphics_items_icons_everstone_sheet, gFile_graphics_items_icon_palettes_everstone_palette},
    [ITEM_FOCUS_BAND] = {gFile_graphics_items_icons_focus_band_sheet, gFile_graphics_items_icon_palettes_focus_band_palette},
    [ITEM_LUCKY_EGG] = {gFile_graphics_items_icons_lucky_egg_sheet, gFile_graphics_items_icon_palettes_lucky_egg_palette},
    [ITEM_SCOPE_LENS] = {gFile_graphics_items_icons_scope_lens_sheet, gFile_graphics_items_icon_palettes_scope_lens_palette},
    [ITEM_METAL_COAT] = {gFile_graphics_items_icons_metal_coat_sheet, gFile_graphics_items_icon_palettes_metal_coat_palette},
    [ITEM_LEFTOVERS] = {gFile_graphics_items_icons_leftovers_sheet, gFile_graphics_items_icon_palettes_leftovers_palette},
    [ITEM_DRAGON_SCALE] = {gFile_graphics_items_icons_dragon_scale_sheet, gFile_graphics_items_icon_palettes_dragon_scale_palette},
    [ITEM_LIGHT_BALL] = {gFile_graphics_items_icons_light_ball_sheet, gFile_graphics_items_icon_palettes_light_ball_palette},
    [ITEM_SOFT_SAND] = {gFile_graphics_items_icons_soft_sand_sheet, gFile_graphics_items_icon_palettes_soft_sand_palette},
    [ITEM_HARD_STONE] = {gFile_graphics_items_icons_hard_stone_sheet, gFile_graphics_items_icon_palettes_hard_stone_palette},
    [ITEM_MIRACLE_SEED] = {gFile_graphics_items_icons_miracle_seed_sheet, gFile_graphics_items_icon_palettes_miracle_seed_palette},
    [ITEM_BLACK_GLASSES] = {gFile_graphics_items_icons_black_glasses_sheet, gFile_graphics_items_icon_palettes_black_type_enhancing_item_palette},
    [ITEM_BLACK_BELT] = {gFile_graphics_items_icons_black_belt_sheet, gFile_graphics_items_icon_palettes_black_type_enhancing_item_palette},
    [ITEM_MAGNET] = {gFile_graphics_items_icons_magnet_sheet, gFile_graphics_items_icon_palettes_magnet_palette},
    [ITEM_MYSTIC_WATER] = {gFile_graphics_items_icons_mystic_water_sheet, gFile_graphics_items_icon_palettes_mystic_water_palette},
    [ITEM_SHARP_BEAK] = {gFile_graphics_items_icons_sharp_beak_sheet, gFile_graphics_items_icon_palettes_sharp_beak_palette},
    [ITEM_POISON_BARB] = {gFile_graphics_items_icons_poison_barb_sheet, gFile_graphics_items_icon_palettes_poison_barb_palette},
    [ITEM_NEVER_MELT_ICE] = {gFile_graphics_items_icons_never_melt_ice_sheet, gFile_graphics_items_icon_palettes_never_melt_ice_palette},
    [ITEM_SPELL_TAG] = {gFile_graphics_items_icons_spell_tag_sheet, gFile_graphics_items_icon_palettes_spell_tag_palette},
    [ITEM_TWISTED_SPOON] = {gFile_graphics_items_icons_twisted_spoon_sheet, gFile_graphics_items_icon_palettes_twisted_spoon_palette},
    [ITEM_CHARCOAL] = {gFile_graphics_items_icons_charcoal_sheet, gFile_graphics_items_icon_palettes_charcoal_palette},
    [ITEM_DRAGON_FANG] = {gFile_graphics_items_icons_dragon_fang_sheet, gFile_graphics_items_icon_palettes_dragon_fang_palette},
    [ITEM_SILK_SCARF] = {gFile_graphics_items_icons_silk_scarf_sheet, gFile_graphics_items_icon_palettes_silk_scarf_palette},
    [ITEM_UP_GRADE] = {gFile_graphics_items_icons_up_grade_sheet, gFile_graphics_items_icon_palettes_up_grade_palette},
    [ITEM_SHELL_BELL] = {gFile_graphics_items_icons_shell_bell_sheet, gFile_graphics_items_icon_palettes_shell_palette},
    [ITEM_SEA_INCENSE] = {gFile_graphics_items_icons_sea_incense_sheet, gFile_graphics_items_icon_palettes_sea_incense_palette},
    [ITEM_LAX_INCENSE] = {gFile_graphics_items_icons_lax_incense_sheet, gFile_graphics_items_icon_palettes_lax_incense_palette},
    [ITEM_LUCKY_PUNCH] = {gFile_graphics_items_icons_lucky_punch_sheet, gFile_graphics_items_icon_palettes_lucky_punch_palette},
    [ITEM_METAL_POWDER] = {gFile_graphics_items_icons_metal_powder_sheet, gFile_graphics_items_icon_palettes_metal_powder_palette},
    [ITEM_THICK_CLUB] = {gFile_graphics_items_icons_thick_club_sheet, gFile_graphics_items_icon_palettes_thick_club_palette},
    [ITEM_STICK] = {gFile_graphics_items_icons_stick_sheet, gFile_graphics_items_icon_palettes_stick_palette},
    [ITEM_0E2] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E3] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E4] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E5] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E6] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E7] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E8] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0E9] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0EA] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0EB] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0EC] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0ED] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0EE] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0EF] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F0] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F1] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F2] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F3] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F4] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F5] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F6] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F7] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F8] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0F9] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0FA] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0FB] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0FC] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_0FD] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_RED_SCARF] = {gFile_graphics_items_icons_scarf_sheet, gFile_graphics_items_icon_palettes_red_scarf_palette},
    [ITEM_BLUE_SCARF] = {gFile_graphics_items_icons_scarf_sheet, gFile_graphics_items_icon_palettes_blue_scarf_palette},
    [ITEM_PINK_SCARF] = {gFile_graphics_items_icons_scarf_sheet, gFile_graphics_items_icon_palettes_pink_scarf_palette},
    [ITEM_GREEN_SCARF] = {gFile_graphics_items_icons_scarf_sheet, gFile_graphics_items_icon_palettes_green_scarf_palette},
    [ITEM_YELLOW_SCARF] = {gFile_graphics_items_icons_scarf_sheet, gFile_graphics_items_icon_palettes_yellow_scarf_palette},
    [ITEM_MACH_BIKE] = {gFile_graphics_items_icons_mach_bike_sheet, gFile_graphics_items_icon_palettes_mach_bike_palette},
    [ITEM_COIN_CASE] = {gFile_graphics_items_icons_coin_case_sheet, gFile_graphics_items_icon_palettes_coin_case_palette},
    [ITEM_ITEMFINDER] = {gFile_graphics_items_icons_itemfinder_sheet, gFile_graphics_items_icon_palettes_itemfinder_palette},
    [ITEM_OLD_ROD] = {gFile_graphics_items_icons_old_rod_sheet, gFile_graphics_items_icon_palettes_old_rod_palette},
    [ITEM_GOOD_ROD] = {gFile_graphics_items_icons_good_rod_sheet, gFile_graphics_items_icon_palettes_good_rod_palette},
    [ITEM_SUPER_ROD] = {gFile_graphics_items_icons_super_rod_sheet, gFile_graphics_items_icon_palettes_super_rod_palette},
    [ITEM_SS_TICKET] = {gFile_graphics_items_icons_ss_ticket_sheet, gFile_graphics_items_icon_palettes_ss_ticket_palette},
    [ITEM_CONTEST_PASS] = {gFile_graphics_items_icons_contest_pass_sheet, gFile_graphics_items_icon_palettes_contest_pass_palette},
    [ITEM_10B] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_WAILMER_PAIL] = {gFile_graphics_items_icons_wailmer_pail_sheet, gFile_graphics_items_icon_palettes_wailmer_pail_palette},
    [ITEM_DEVON_GOODS] = {gFile_graphics_items_icons_devon_goods_sheet, gFile_graphics_items_icon_palettes_devon_goods_palette},
    [ITEM_SOOT_SACK] = {gFile_graphics_items_icons_soot_sack_sheet, gFile_graphics_items_icon_palettes_soot_sack_palette},
    [ITEM_BASEMENT_KEY] = {gFile_graphics_items_icons_basement_key_sheet, gFile_graphics_items_icon_palettes_old_key_palette},
    [ITEM_ACRO_BIKE] = {gFile_graphics_items_icons_acro_bike_sheet, gFile_graphics_items_icon_palettes_acro_bike_palette},
    [ITEM_POKEBLOCK_CASE] = {gFile_graphics_items_icons_pokeblock_case_sheet, gFile_graphics_items_icon_palettes_pokeblock_case_palette},
    [ITEM_LETTER] = {gFile_graphics_items_icons_letter_sheet, gFile_graphics_items_icon_palettes_lava_cookie_and_letter_palette},
    [ITEM_EON_TICKET] = {gFile_graphics_items_icons_eon_ticket_sheet, gFile_graphics_items_icon_palettes_eon_ticket_palette},
    [ITEM_RED_ORB] = {gFile_graphics_items_icons_orb_sheet, gFile_graphics_items_icon_palettes_red_orb_palette},
    [ITEM_BLUE_ORB] = {gFile_graphics_items_icons_orb_sheet, gFile_graphics_items_icon_palettes_blue_orb_palette},
    [ITEM_SCANNER] = {gFile_graphics_items_icons_scanner_sheet, gFile_graphics_items_icon_palettes_scanner_palette},
    [ITEM_GO_GOGGLES] = {gFile_graphics_items_icons_go_goggles_sheet, gFile_graphics_items_icon_palettes_go_goggles_palette},
    [ITEM_METEORITE] = {gFile_graphics_items_icons_meteorite_sheet, gFile_graphics_items_icon_palettes_meteorite_palette},
    [ITEM_ROOM_1_KEY] = {gFile_graphics_items_icons_room1_key_sheet, gFile_graphics_items_icon_palettes_key_palette},
    [ITEM_ROOM_2_KEY] = {gFile_graphics_items_icons_room2_key_sheet, gFile_graphics_items_icon_palettes_key_palette},
    [ITEM_ROOM_4_KEY] = {gFile_graphics_items_icons_room4_key_sheet, gFile_graphics_items_icon_palettes_key_palette},
    [ITEM_ROOM_6_KEY] = {gFile_graphics_items_icons_room6_key_sheet, gFile_graphics_items_icon_palettes_key_palette},
    [ITEM_STORAGE_KEY] = {gFile_graphics_items_icons_storage_key_sheet, gFile_graphics_items_icon_palettes_old_key_palette},
    [ITEM_ROOT_FOSSIL] = {gFile_graphics_items_icons_root_fossil_sheet, gFile_graphics_items_icon_palettes_hoenn_fossil_palette},
    [ITEM_CLAW_FOSSIL] = {gFile_graphics_items_icons_claw_fossil_sheet, gFile_graphics_items_icon_palettes_hoenn_fossil_palette},
    [ITEM_DEVON_SCOPE] = {gFile_graphics_items_icons_devon_scope_sheet, gFile_graphics_items_icon_palettes_devon_scope_palette},
    [ITEM_TM01] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fighting_tm_hm_palette},
    [ITEM_TM02] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_dragon_tm_hm_palette},
    [ITEM_TM03] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_water_tm_hm_palette},
    [ITEM_TM04] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_psychic_tm_hm_palette},
    [ITEM_TM05] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM06] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_poison_tm_hm_palette},
    [ITEM_TM07] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_ice_tm_hm_palette},
    [ITEM_TM08] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fighting_tm_hm_palette},
    [ITEM_TM09] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_grass_tm_hm_palette},
    [ITEM_TM10] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM11] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fire_tm_hm_palette},
    [ITEM_TM12] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_dark_tm_hm_palette},
    [ITEM_TM13] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_ice_tm_hm_palette},
    [ITEM_TM14] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_ice_tm_hm_palette},
    [ITEM_TM15] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM16] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_psychic_tm_hm_palette},
    [ITEM_TM17] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM18] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_water_tm_hm_palette},
    [ITEM_TM19] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_grass_tm_hm_palette},
    [ITEM_TM20] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM21] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM22] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_grass_tm_hm_palette},
    [ITEM_TM23] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_steel_tm_hm_palette},
    [ITEM_TM24] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_electric_tm_hm_palette},
    [ITEM_TM25] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_electric_tm_hm_palette},
    [ITEM_TM26] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_ground_tm_hm_palette},
    [ITEM_TM27] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM28] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_ground_tm_hm_palette},
    [ITEM_TM29] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_psychic_tm_hm_palette},
    [ITEM_TM30] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_ghost_tm_hm_palette},
    [ITEM_TM31] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fighting_tm_hm_palette},
    [ITEM_TM32] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM33] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_psychic_tm_hm_palette},
    [ITEM_TM34] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_electric_tm_hm_palette},
    [ITEM_TM35] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fire_tm_hm_palette},
    [ITEM_TM36] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_poison_tm_hm_palette},
    [ITEM_TM37] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_rock_tm_hm_palette},
    [ITEM_TM38] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fire_tm_hm_palette},
    [ITEM_TM39] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_rock_tm_hm_palette},
    [ITEM_TM40] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_flying_tm_hm_palette},
    [ITEM_TM41] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_dark_tm_hm_palette},
    [ITEM_TM42] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM43] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM44] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_psychic_tm_hm_palette},
    [ITEM_TM45] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_TM46] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_dark_tm_hm_palette},
    [ITEM_TM47] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_steel_tm_hm_palette},
    [ITEM_TM48] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_psychic_tm_hm_palette},
    [ITEM_TM49] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_dark_tm_hm_palette},
    [ITEM_TM50] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fire_tm_hm_palette},
    [ITEM_HM01] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_HM02] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_flying_tm_hm_palette},
    [ITEM_HM03] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_water_tm_hm_palette},
    [ITEM_HM04] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_HM05] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_normal_tm_hm_palette},
    [ITEM_HM06] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_fighting_tm_hm_palette},
    [ITEM_HM07] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_water_tm_hm_palette},
    [ITEM_HM08] = {gFile_graphics_items_icons_tm_sheet, gFile_graphics_items_icon_palettes_water_tm_hm_palette},
    [ITEM_15B] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_15C] = {gFile_graphics_items_icons_question_mark_sheet, gFile_graphics_items_icon_palettes_question_mark_palette},
    [ITEM_OAKS_PARCEL] = {gFile_graphics_items_icons_oaks_parcel_sheet, gFile_graphics_items_icon_palettes_oaks_parcel_palette},
    [ITEM_POKE_FLUTE] = {gFile_graphics_items_icons_poke_flute_sheet, gFile_graphics_items_icon_palettes_poke_flute_palette},
    [ITEM_SECRET_KEY] = {gFile_graphics_items_icons_secret_key_sheet, gFile_graphics_items_icon_palettes_secret_key_palette},
    [ITEM_BIKE_VOUCHER] = {gFile_graphics_items_icons_bike_voucher_sheet, gFile_graphics_items_icon_palettes_bike_voucher_palette},
    [ITEM_GOLD_TEETH] = {gFile_graphics_items_icons_gold_teeth_sheet, gFile_graphics_items_icon_palettes_gold_teeth_palette},
    [ITEM_OLD_AMBER] = {gFile_graphics_items_icons_old_amber_sheet, gFile_graphics_items_icon_palettes_old_amber_palette},
    [ITEM_CARD_KEY] = {gFile_graphics_items_icons_card_key_sheet, gFile_graphics_items_icon_palettes_card_key_palette},
    [ITEM_LIFT_KEY] = {gFile_graphics_items_icons_lift_key_sheet, gFile_graphics_items_icon_palettes_key_palette},
    [ITEM_HELIX_FOSSIL] = {gFile_graphics_items_icons_helix_fossil_sheet, gFile_graphics_items_icon_palettes_kanto_fossil_palette},
    [ITEM_DOME_FOSSIL] = {gFile_graphics_items_icons_dome_fossil_sheet, gFile_graphics_items_icon_palettes_kanto_fossil_palette},
    [ITEM_SILPH_SCOPE] = {gFile_graphics_items_icons_silph_scope_sheet, gFile_graphics_items_icon_palettes_silph_scope_palette},
    [ITEM_BICYCLE] = {gFile_graphics_items_icons_bicycle_sheet, gFile_graphics_items_icon_palettes_bicycle_palette},
    [ITEM_TOWN_MAP] = {gFile_graphics_items_icons_town_map_sheet, gFile_graphics_items_icon_palettes_town_map_palette},
    [ITEM_VS_SEEKER] = {gFile_graphics_items_icons_vs_seeker_sheet, gFile_graphics_items_icon_palettes_vs_seeker_palette},
    [ITEM_FAME_CHECKER] = {gFile_graphics_items_icons_fame_checker_sheet, gFile_graphics_items_icon_palettes_fame_checker_palette},
    [ITEM_TM_CASE] = {gFile_graphics_items_icons_tm_case_sheet, gFile_graphics_items_icon_palettes_tm_case_palette},
    [ITEM_BERRY_POUCH] = {gFile_graphics_items_icons_berry_pouch_sheet, gFile_graphics_items_icon_palettes_berry_pouch_palette},
    [ITEM_TEACHY_TV] = {gFile_graphics_items_icons_teachy_tv_sheet, gFile_graphics_items_icon_palettes_teachy_tv_palette},
    [ITEM_TRI_PASS] = {gFile_graphics_items_icons_tri_pass_sheet, gFile_graphics_items_icon_palettes_tri_pass_palette},
    [ITEM_RAINBOW_PASS] = {gFile_graphics_items_icons_rainbow_pass_sheet, gFile_graphics_items_icon_palettes_rainbow_pass_palette},
    [ITEM_TEA] = {gFile_graphics_items_icons_tea_sheet, gFile_graphics_items_icon_palettes_tea_palette},
    [ITEM_MYSTIC_TICKET] = {gFile_graphics_items_icons_mystic_ticket_sheet, gFile_graphics_items_icon_palettes_mystic_ticket_palette},
    [ITEM_AURORA_TICKET] = {gFile_graphics_items_icons_aurora_ticket_sheet, gFile_graphics_items_icon_palettes_aurora_ticket_palette},
    [ITEM_POWDER_JAR] = {gFile_graphics_items_icons_powder_jar_sheet, gFile_graphics_items_icon_palettes_powder_jar_palette},
    [ITEM_RUBY] = {gFile_graphics_items_icons_gem_sheet, gFile_graphics_items_icon_palettes_ruby_palette},
    [ITEM_SAPPHIRE] = {gFile_graphics_items_icons_gem_sheet, gFile_graphics_items_icon_palettes_sapphire_palette},
    [ITEMS_COUNT] = {gFile_graphics_items_icons_return_to_field_arrow_sheet, gFile_graphics_items_icon_palettes_return_to_field_arrow_palette}
};


void ResetItemMenuIconState(void)
{
    u16 i;

    for (i = 0; i < SPR_COUNT; i++)
        sItemMenuIconSpriteIds[i] = SPRITE_NONE;
}

void CreateBagSprite(u8 animNum)
{
    sItemMenuIconSpriteIds[SPR_BAG] = CreateSprite(&sSpriteTemplate_Bag, 40, 68, 0);
    SetBagVisualPocketId(animNum);
}

void SetBagVisualPocketId(u8 animNum)
{
    struct Sprite *sprite = &gSprites[sItemMenuIconSpriteIds[SPR_BAG]];
    sprite->y2 = -5;
    sprite->callback = SpriteCB_BagVisualSwitchingPockets;
    StartSpriteAnim(sprite, animNum);
}

static void SpriteCB_BagVisualSwitchingPockets(struct Sprite *sprite)
{
    if (sprite->y2 != 0)
        sprite->y2++;
    else
        sprite->callback = SpriteCallbackDummy;
}

void ShakeBagSprite(void)
{
    struct Sprite *sprite = &gSprites[sItemMenuIconSpriteIds[SPR_BAG]];
    if (sprite->affineAnimEnded)
    {
        StartSpriteAffineAnim(sprite, AFFINEANIM_BAG_SHAKE);
        sprite->callback = SpriteCB_ShakeBagSprite;
    }
}

static void SpriteCB_ShakeBagSprite(struct Sprite *sprite)
{
    if (sprite->affineAnimEnded)
    {
        StartSpriteAffineAnim(sprite, AFFINEANIM_BAG_IDLE);
        sprite->callback = SpriteCallbackDummy;
    }
}

void CreateSwapLine(void)
{
    u8 i;
    u8 * spriteIds = &sItemMenuIconSpriteIds[SPR_SWAP_LINE_START];

    for (i = 0; i < NUM_SWAP_LINE_SPRITES; i++)
    {
        spriteIds[i] = CreateSprite(&sSpriteTemplate_SwapLine, i * 16 + 96, 7, 0);
        switch (i)
        {
        case 0:
            // ANIM_SWAP_LINE_START, by default
            break;
        case NUM_SWAP_LINE_SPRITES - 1:
            StartSpriteAnim(&gSprites[spriteIds[i]], ANIM_SWAP_LINE_END);
            break;
        default:
            StartSpriteAnim(&gSprites[spriteIds[i]], ANIM_SWAP_LINE_MID);
            break;
        }
        gSprites[spriteIds[i]].invisible = TRUE;
    }
}

void SetSwapLineInvisibility(bool8 invisible)
{
    u8 i;
    u8 * spriteIds = &sItemMenuIconSpriteIds[SPR_SWAP_LINE_START];

    for (i = 0; i < NUM_SWAP_LINE_SPRITES; i++)
        gSprites[spriteIds[i]].invisible = invisible;
}

void UpdateSwapLinePos(s16 x, u16 y)
{
    u8 i;
    u8 * spriteIds = &sItemMenuIconSpriteIds[SPR_SWAP_LINE_START];

    for (i = 0; i < NUM_SWAP_LINE_SPRITES; i++)
    {
        gSprites[spriteIds[i]].x2 = x;
        gSprites[spriteIds[i]].y = y + 7;
    }
}

static bool8 TryAllocItemIconTilesBuffers(void)
{
    void ** ptr1, ** ptr2;

    ptr1 = &sItemIconTilesBuffer;
    *ptr1 = Alloc(0x120);
    if (*ptr1 == NULL)
        return FALSE;
    ptr2 = &sItemIconTilesBufferPadded;
    *ptr2 = AllocZeroed(0x200);
    if (*ptr2 == NULL)
    {
        Free(*ptr1);
        return FALSE;
    }
    return TRUE;
}

void CopyItemIconPicTo4x4Buffer(const void *src, void *dest)
{
    u8 i;

    for (i = 0; i < 3; i++)
        CpuCopy16(src + 0x60 * i, dest + 0x80 * i, 0x60);
}

u8 AddItemIconObject(u16 tilesTag, u16 paletteTag, u16 itemId)
{
    struct SpriteTemplate template;
    struct SpriteSheet spriteSheet;
    struct CompressedSpritePalette spritePalette;
    u8 spriteId;

    if (!TryAllocItemIconTilesBuffers())
        return MAX_SPRITES;

    LZDecompressWram(GetItemIconGfxPtr(itemId, 0), sItemIconTilesBuffer);
    CopyItemIconPicTo4x4Buffer(sItemIconTilesBuffer, sItemIconTilesBufferPadded);
    spriteSheet.data = sItemIconTilesBufferPadded;
    spriteSheet.size = 0x200;
    spriteSheet.tag = tilesTag;
    LoadSpriteSheet(&spriteSheet);

    spritePalette.data = GetItemIconGfxPtr(itemId, 1);
    spritePalette.tag = paletteTag;
    LoadCompressedSpritePalette(&spritePalette);

    CpuCopy16(&sSpriteTemplate_ItemIcon, &template, sizeof(struct SpriteTemplate));
    template.tileTag = tilesTag;
    template.paletteTag = paletteTag;
    spriteId = CreateSprite(&template, 0, 0, 0);

    Free(sItemIconTilesBuffer);
    Free(sItemIconTilesBufferPadded);
    return spriteId;
}

u8 AddItemIconObjectWithCustomObjectTemplate(const struct SpriteTemplate * origTemplate, u16 tilesTag, u16 paletteTag, u16 itemId)
{
    struct SpriteTemplate template;
    struct SpriteSheet spriteSheet;
    struct CompressedSpritePalette spritePalette;
    u8 spriteId;

    if (!TryAllocItemIconTilesBuffers())
        return MAX_SPRITES;

    LZDecompressWram(GetItemIconGfxPtr(itemId, 0), sItemIconTilesBuffer);
    CopyItemIconPicTo4x4Buffer(sItemIconTilesBuffer, sItemIconTilesBufferPadded);
    spriteSheet.data = sItemIconTilesBufferPadded;
    spriteSheet.size = 0x200;
    spriteSheet.tag = tilesTag;
    LoadSpriteSheet(&spriteSheet);

    spritePalette.data = GetItemIconGfxPtr(itemId, 1);
    spritePalette.tag = paletteTag;
    LoadCompressedSpritePalette(&spritePalette);

    CpuCopy16(origTemplate, &template, sizeof(struct SpriteTemplate));
    template.tileTag = tilesTag;
    template.paletteTag = paletteTag;
    spriteId = CreateSprite(&template, 0, 0, 0);

    Free(sItemIconTilesBuffer);
    Free(sItemIconTilesBufferPadded);
    return spriteId;
}

void CreateItemMenuIcon(u16 itemId, u8 idx)
{
    u8 * spriteIds = &sItemMenuIconSpriteIds[SPR_ITEM_ICON];
    u8 spriteId;

    if (spriteIds[idx] == SPRITE_NONE)
    {
        // Either TAG_ITEM_ICON or TAG_ITEM_ICON_ALT
        FreeSpriteTilesByTag(TAG_ITEM_ICON + idx);
        FreeSpritePaletteByTag(TAG_ITEM_ICON + idx);
        spriteId = AddItemIconObject(TAG_ITEM_ICON + idx, TAG_ITEM_ICON + idx, itemId);
        if (spriteId != MAX_SPRITES)
        {
            spriteIds[idx] = spriteId;
            gSprites[spriteId].x2 = 24;
            gSprites[spriteId].y2 = 140;
        }
    }
}

void DestroyItemMenuIcon(u8 idx)
{
    u8 * spriteIds = &sItemMenuIconSpriteIds[SPR_ITEM_ICON];

    if (spriteIds[idx] != SPRITE_NONE)
    {
        DestroySpriteAndFreeResources(&gSprites[spriteIds[idx]]);
        spriteIds[idx] = SPRITE_NONE;
    }
}

const void *GetItemIconGfxPtr(u16 itemId, u8 attrId)
{
    if (itemId > ITEMS_COUNT)
        itemId = ITEM_NONE;
    return sItemIconTable[itemId][attrId];
}

void CreateBerryPouchItemIcon(u16 itemId, u8 idx)
{
    u8 * spriteIds = &sItemMenuIconSpriteIds[SPR_ITEM_ICON];
    u8 spriteId;

    if (spriteIds[idx] == SPRITE_NONE)
    {
        // Either TAG_ITEM_ICON or TAG_ITEM_ICON_ALT
        FreeSpriteTilesByTag(TAG_ITEM_ICON + idx);
        FreeSpritePaletteByTag(TAG_ITEM_ICON + idx);
        spriteId = AddItemIconObject(TAG_ITEM_ICON + idx, TAG_ITEM_ICON + idx, itemId);
        if (spriteId != MAX_SPRITES)
        {
            spriteIds[idx] = spriteId;
            gSprites[spriteId].x2 = 24;
            gSprites[spriteId].y2 = 147; // This value is the only difference from CreateItemMenuIcon
        }
    }
}
