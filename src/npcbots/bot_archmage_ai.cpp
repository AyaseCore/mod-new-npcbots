#include "bot_ai.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuras.h"
/*
Archmage NpcBot (by Trickerer onlysuffering@gmail.com)
Description:
Archmage (Warcraft III tribute)
Abilities:
1) Fireball: main attack, single target, no mana cost
2) Blizzard: typical blizzard
3) Summon Water Elemental: summons a water elemental to attack archmage's enemies
Complete - 75%
TODO: mass tele
*/

enum ArchmageBaseSpells
{
    MAIN_ATTACK_1           = SPELL_FIREBALL,
    BLIZZARD_1              = SPELL_BLIZZARD,
    SUMMON_WATER_ELEMENTAL_1= SPELL_SUMMON_WATER_ELEMENTAL
};
enum ArchmagePassives
{
    BRILLIANCE_AURA         = SPELL_BRILLIANCE_AURA
};
enum ArchmageSpecial
{
    MH_ATTACK_ANIM          = SPELL_ATTACK_MELEE_1H,

    SUMMON_ELEM_COST        = 125 * 5,

    ARCHMAGE_MOUNTID        = 2402
};

class archmage_bot : public CreatureScript
{
public:
    archmage_bot() : CreatureScript("archmage_bot") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new archmage_botAI(creature);
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        return creature->GetBotAI()->OnGossipHello(player, 0);
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (bot_ai* ai = creature->GetBotAI())
            return ai->OnGossipSelect(player, creature, sender, action);
        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, char const* code)
    {
        if (bot_ai* ai = creature->GetBotAI())
            return ai->OnGossipSelectCode(player, creature, sender, action, code);
        return true;
    }

    struct archmage_botAI : public bot_ai
    {
        archmage_botAI(Creature* creature) : bot_ai(creature)
        {
            _botclass = BOT_CLASS_ARCHMAGE;

            //archmage immunities
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_POSSESS, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_SILENCE, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_SHAPESHIFT, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_TRANSFORM, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_BLOCK_SPELL_FAMILY, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SILENCE, true);
        }

        bool doCast(Unit* victim, uint32 spellId)
        {
            if (CheckBotCast(victim, spellId) != SPELL_CAST_OK)
                return false;
            return bot_ai::doCast(victim, spellId);
        }

        void StartAttack(Unit* u, bool force = false)
        {
            if (GetBotCommandState() == COMMAND_ATTACK && !force) return;
            SetBotCommandState(COMMAND_ATTACK);
            OnStartAttack(u);
            GetInPosition(force, u);
        }

        void EnterCombat(Unit* u) { bot_ai::EnterCombat(u); }
        void KilledUnit(Unit* u) { bot_ai::KilledUnit(u); }
        void EnterEvadeMode() { bot_ai::EnterEvadeMode(); }
        void MoveInLineOfSight(Unit* u) { bot_ai::MoveInLineOfSight(u); }
        void JustDied(Unit* u) { UnsummonAll(); bot_ai::JustDied(u); }
        void DoNonCombatActions(uint32 /*diff*/) { }

        void CheckAura(uint32 diff)
        {
            if (checkAuraTimer > diff || GC_Timer > diff || IsCasting())
                return;

            checkAuraTimer = 10000;

            if (!me->HasAura(BRILLIANCE_AURA, me->GetGUID()))
                RefreshAura(BRILLIANCE_AURA);
        }

        void UpdateAI(uint32 diff)
        {
            if (!me->IsMounted())
                me->Mount(ARCHMAGE_MOUNTID);

            if (!GlobalUpdate(diff))
                return;

            CheckAura(diff);

            if (IsPotionReady())
            {
                if (me->GetPower(POWER_MANA) < SUMMON_ELEM_COST)
                    DrinkPotion(true);
                else if (GetHealthPCT(me) < 50)
                    DrinkPotion(false);
            }

            //pet is killed or unreachable
            if (IsSpellReady(SUMMON_WATER_ELEMENTAL_1, diff, false) && me->GetPower(POWER_MANA) >= SUMMON_ELEM_COST && !IsCasting() &&
                (IAmFree() || master->IsInCombat()/* || !master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING)*/) &&
                (!botPet || me->GetDistance2d(botPet) > sWorld->GetMaxVisibleDistanceOnContinents()))
            {
                me->CastSpell(me, GetSpell(SUMMON_WATER_ELEMENTAL_1), false);
                return;
            }

            if (!CheckAttackTarget())
                return;

            if (IsCasting())
                return;

            Attack(diff);
        }

        void Attack(uint32 diff)
        {
            StartAttack(opponent, IsMelee());

            if (!HasRole(BOT_ROLE_DPS))
                return;

            if (GC_Timer > diff)
                return;

            if (CanAffectVictim(SPELL_SCHOOL_MASK_FROST))
            {
                //Blizzard
                if (IsSpellReady(BLIZZARD_1, diff) && !JumpingOrFalling() && Rand() < 50)
                {
                    if (Unit* blizztarget = FindAOETarget(CalcSpellMaxRange(BLIZZARD_1)))
                    {
                        if (doCast(blizztarget, GetSpell(BLIZZARD_1)))
                            return;
                    }

                    SetSpellCooldown(BLIZZARD_1, 1000); //fail
                }
            }

            if (IsSpellReady(MAIN_ATTACK_1, diff))
            {
                if (doCast(opponent, GetSpell(MAIN_ATTACK_1)))
                    return;
            }
        }

        void ApplyClassDamageMultiplierSpell(int32& damage, SpellNonMeleeDamage& damageinfo, SpellInfo const* spellInfo, WeaponAttackType /*attackType*/, bool crit) const
        {
            uint32 baseId = spellInfo->GetFirstRankSpell()->Id;
            uint8 lvl = me->getLevel();
            float fdamage = float(damage);

            //apply bonus damage mods
            float pctbonus = 1.0f;
            if (crit)
                pctbonus *= 1.333f;

            if (baseId == MAIN_ATTACK_1 || baseId == BLIZZARD_1)
                fdamage += me->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_MAGIC) * (spellInfo->Effects[0].BonusMultiplier - 1.f) * me->CalculateDefaultCoefficient(spellInfo, SPELL_DIRECT_DAMAGE) * me->CalculateLevelPenalty(spellInfo);

            damage = int32(fdamage * pctbonus);
        }

        void OnClassSpellGo(SpellInfo const* spellInfo)
        {
            uint32 baseId = spellInfo->GetFirstRankSpell()->Id;

            if (baseId == MAIN_ATTACK_1 || baseId == BLIZZARD_1)
                GC_Timer = me->GetAttackTime(BASE_ATTACK);

            if (baseId == MAIN_ATTACK_1)
                me->CastSpell(me, MH_ATTACK_ANIM, true);

            if (baseId == SUMMON_WATER_ELEMENTAL_1)
                SummonBotPet();
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (!spell->_IsPositiveSpell() && spell->GetMaxDuration() >= 1000 && caster->IsControlledByPlayer())
            {
                //bots of W3 classes will not be easily CCed
                if (spell->HasAura(SPELL_AURA_MOD_STUN) || spell->HasAura(SPELL_AURA_MOD_CONFUSE) ||
                    spell->HasAura(SPELL_AURA_MOD_PACIFY) || spell->HasAura(SPELL_AURA_MOD_ROOT))
                {
                    if (Aura* cont = me->GetAura(spell->Id, caster->GetGUID()))
                    {
                        if (AuraApplication const* aurApp = cont->GetApplicationOfTarget(me->GetGUID()))
                        {
                            if (!aurApp->IsPositive())
                            {
                                int32 dur = std::max<int32>(cont->GetMaxDuration() / 3, 1000);
                                cont->SetDuration(dur);
                                cont->SetMaxDuration(dur);
                            }
                        }
                    }
                }
            }

            OnSpellHit(caster, spell);
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType damageType)
        {
            bot_ai::DamageDealt(victim, damage, damageType);
        }

        void DamageTaken(Unit* u, uint32& damage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask)
        {
            if (!u->IsInCombat() && !me->IsInCombat())
                return;
            OnOwnerDamagedBy(u);
        }

        void OwnerAttackedBy(Unit* u)
        {
            OnOwnerDamagedBy(u);
        }

        void SummonBotPet()
        {
            if (botPet)
                UnsummonAll();

            uint32 entry = BOT_PET_AWATER_ELEMENTAL;

            Position pos;

            //water elemetal 1 minute duration
            Creature* myPet = me->SummonCreature(entry, *me, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, IAmFree() ? 3600000 : 60000);
            myPet->IsAIEnabled = false;
            me->GetNearPoint(myPet, pos.m_positionX, pos.m_positionY, pos.m_positionZ, 0, 2, me->GetOrientation());
            myPet->GetMotionMaster()->MovePoint(me->GetMapId(), pos);
            myPet->SetCreatorGUID(master->GetGUID());
            myPet->SetOwnerGUID(me->GetGUID());
            myPet->setFaction(master->getFaction());
            myPet->m_ControlledByPlayer = !IAmFree();
            myPet->SetPvP(me->IsPvP());
            myPet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
            myPet->SetByteValue(UNIT_FIELD_BYTES_2, 1, master->GetByteValue(UNIT_FIELD_BYTES_2, 1));
            myPet->SetUInt32Value(UNIT_CREATED_BY_SPELL, SUMMON_WATER_ELEMENTAL_1);

            botPet = myPet;
            myPet->IsAIEnabled = true;
        }

        void UnsummonAll()
        {
            if (botPet)
                botPet->ToTempSummon()->UnSummon();
        }

        void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/)
        {
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            //sLog->outError("SummonedCreatureDespawn: %s's %s", me->GetName().c_str(), summon->GetName().c_str());
            if (summon == botPet)
                botPet = NULL;
        }

        uint32 GetAIMiscValue(uint32 data) const
        {
            switch (data)
            {
                case BOTAI_MISC_PET_TYPE:
                    return BOT_PET_AWATER_ELEMENTAL;
                default:
                    return 0;
            }
        }

        void CheckAttackState()
        {
            if (me->GetVictim())
            {
                //if (HasRole(BOT_ROLE_DPS))
                //    DoMeleeAttackIfReady();
            }
            else
                Evade();
        }

        void Reset()
        {
            UnsummonAll();

            checkAuraTimer = 0;

            DefaultInit();
        }

        void ReduceCD(uint32 diff)
        {
            if (checkAuraTimer > diff)              checkAuraTimer -= diff;
        }

        void InitPowers()
        {
            me->setPowerType(POWER_MANA);
        }

        void InitSpells()
        {
            InitSpellMap(MAIN_ATTACK_1, true, false);
            InitSpellMap(BLIZZARD_1, true, false);
            InitSpellMap(SUMMON_WATER_ELEMENTAL_1, true, false);
        }

        void ApplyClassPassives() const
        {
        }

        bool CanUseManually(uint32 basespell) const
        {
            switch (basespell)
            {
                case BLIZZARD_1:
                case SUMMON_WATER_ELEMENTAL_1:
                    return true;
                default:
                    return false;
            }
        }

    private:

        uint32 checkAuraTimer;
    };
};

void AddSC_archmage_bot()
{
    new archmage_bot();
}
