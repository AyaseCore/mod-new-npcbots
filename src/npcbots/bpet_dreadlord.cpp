#include "bot_ai.h"
#include "bpet_ai.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
/*
Dreadlord NpcBot Pets (by Trickerer onlysuffering@gmail.com)
Complete - 100%
TODO:
*/

enum DreadlordPetBaseSpells
{
};

enum DreadlordPetPassives
{
};

enum DreadlordPetSpecial
{
    IMMOLATION_DAMAGE       = 35959,

    INFERNAL_DURATION       = 180000 - 2000 //3 min
};

class dreadlord_pet_bot : public CreatureScript
{
public:
    dreadlord_pet_bot() : CreatureScript("dreadlord_pet_bot") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new dreadlord_botpetAI(creature);
    }

    struct dreadlord_botpetAI : public bot_pet_ai
    {
        dreadlord_botpetAI(Creature* creature) : bot_pet_ai(creature) { }

        void EnterCombat(Unit* u) {bot_pet_ai::EnterCombat(u); }
        void KilledUnit(Unit* u) { bot_pet_ai::KilledUnit(u); }
        void EnterEvadeMode() { bot_pet_ai::EnterEvadeMode(); }
        void MoveInLineOfSight(Unit* u) { bot_pet_ai::MoveInLineOfSight(u); }
        void JustDied(Unit* u) { bot_pet_ai::JustDied(u); }
        void DoNonCombatActions(uint32 /*diff*/) { }

        void StartAttack(Unit* u, bool force = false)
        {
            if (GetBotCommandState() == COMMAND_ATTACK && !force) return;
            SetBotCommandState(COMMAND_ATTACK);
            OnStartAttack(u);
            GetInPosition(force, u);
        }

        void DoPetActions(uint32 diff)
        {
        }

        void UpdateAI(uint32 diff)
        {
            //infernal death & unsummon
            if ((liveTimer += diff) >= INFERNAL_DURATION)
            {
                me->IsAIEnabled = false;
                me->setDeathState(JUST_DIED);
                return;
            }

            if (!GlobalUpdate(diff))
                return;

            if (!me->IsInCombat())
                DoNonCombatActions(diff);

            //CheckDrainMana(diff);

            if (!CheckAttackTarget())
                return;

            if (IsCasting())
                return;

            DoPetAttack(diff);
        }

        void DoPetAttack(uint32 diff)
        {
            StartAttack(opponent, IsPetMelee());
        }

        void ApplyClassDamageMultiplierSpell(int32& damage, SpellNonMeleeDamage& damageinfo, SpellInfo const* spellInfo, WeaponAttackType /*attackType*/, bool crit) const
        {
            uint32 baseId = spellInfo->GetFirstRankSpell()->Id;
            float fdamage = float(damage);

            float pctbonus = 1.0f;
            pctbonus *= 0.5f;

            if (baseId == IMMOLATION_DAMAGE)
                fdamage += me->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_MAGIC) * me->CalculateDefaultCoefficient(spellInfo, SPELL_DIRECT_DAMAGE) * me->CalculateLevelPenalty(spellInfo);

            damage = int32(fdamage * pctbonus);
        }

        void OnPetClassSpellGo(SpellInfo const* /*spellInfo*/)
        {
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            OnSpellHit(caster, spell);
        }

        void SpellHitTarget(Unit* /*target*/, SpellInfo const* /*spell*/)
        {
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType damageType)
        {
            bot_pet_ai::DamageDealt(victim, damage, damageType);
        }

        void DamageTaken(Unit* u, uint32& /*damage*/, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask)
        {
            if (!u->IsInCombat() && !me->IsInCombat())
                return;
            OnOwnerDamagedBy(u);
        }

        void OwnerAttackedBy(Unit* u)
        {
            OnOwnerDamagedBy(u);
        }

        void Reset()
        {
            liveTimer = 0;
        }

        void InitPetSpells()
        {
        }

        void ApplyPetPassives() const
        {
        }

    private:
        uint32 liveTimer;
    };
};

void AddSC_dreadlord_bot_pets()
{
    new dreadlord_pet_bot();
}
