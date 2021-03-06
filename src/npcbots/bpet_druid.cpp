#include "bot_ai.h"
#include "bpet_ai.h"
#include "ScriptMgr.h"
/*
Druid NpcBot Pets (by Trickerer onlysuffering@gmail.com)
Complete - 100%
TODO:
*/

enum DruidPetBaseSpells
{
};

enum DruidPetPassives
{
};

enum DruidPetSpecial
{
    TREANT_DURATION         = 30000
};

class druid_pet_bot : public CreatureScript
{
public:
    druid_pet_bot() : CreatureScript("druid_pet_bot") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new druid_botpetAI(creature);
    }

    struct druid_botpetAI : public bot_pet_ai
    {
        druid_botpetAI(Creature* creature) : bot_pet_ai(creature) { }

        void EnterCombat(Unit* u) {bot_pet_ai::EnterCombat(u); }
        void KilledUnit(Unit* u) { bot_pet_ai::KilledUnit(u); }
        void EnterEvadeMode() { bot_pet_ai::EnterEvadeMode(); }
        void MoveInLineOfSight(Unit* u) { bot_pet_ai::MoveInLineOfSight(u); }
        void JustDied(Unit* u) { me->IsAIEnabled = false; me->ToTempSummon()->UnSummon(5000); bot_pet_ai::JustDied(u); }
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
            if ((liveTimer += diff) >= TREANT_DURATION)
            {
                me->IsAIEnabled = false;
                me->ToTempSummon()->UnSummon(1);
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

void AddSC_druid_bot_pets()
{
    new druid_pet_bot();
}
