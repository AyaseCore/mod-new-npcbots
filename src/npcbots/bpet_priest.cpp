#include "bot_ai.h"
#include "bpet_ai.h"
#include "ScriptMgr.h"
/*
Priest NpcBot Pets (by Trickerer onlysuffering@gmail.com)
Complete - 100%
TODO:
*/

enum PriestPetBaseSpells
{
    SHADOWCRAWL_1                       = 63619
};

enum PriestPetPassives
{
    MANA_LEECH                          = 28305,
    AVOIDANCE                           = 63623
};

enum PriestPetSpecial
{
    GLYPH_SHADOWFIEND_PROC              = 58227,

    SHADOWFIEND_DURATION                = 15000
};

class priest_pet_bot : public CreatureScript
{
public:
    priest_pet_bot() : CreatureScript("priest_pet_bot") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new priest_botpetAI(creature);
    }

    struct priest_botpetAI : public bot_pet_ai
    {
        priest_botpetAI(Creature* creature) : bot_pet_ai(creature) { }

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
            if ((liveTimer += diff) >= SHADOWFIEND_DURATION)
            {
                me->IsAIEnabled = false;
                me->ToTempSummon()->UnSummon(1);
                return;
            }

            if (!GlobalUpdate(diff))
                return;

            if (!me->IsInCombat())
                DoNonCombatActions(diff);

            DoPetActions(diff);
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

            float dist = me->GetDistance(opponent);
            bool canDPS = petOwner->GetBotAI()->HasRole(BOT_ROLE_DPS);

            if (IsSpellReady(SHADOWCRAWL_1, diff) && canDPS && dist < 30)
            {
                me->CastSpell(opponent, GetSpell(SHADOWCRAWL_1), false);
                SetSpellCooldown(SHADOWCRAWL_1, 6000);
                return;
            }
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
            //Handled by spell scripts
            //if (damage && victim && damageType == DIRECT_DAMAGE)
            //    victim->CastSpell(petOwner, MANA_LEECH_PROC, true);

            bot_pet_ai::DamageDealt(victim, damage, damageType);
        }

        void DamageTaken(Unit* u, uint32& damage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask)
        {
            if (damage >= me->GetHealth())
                petOwner->CastSpell(petOwner, GLYPH_SHADOWFIEND_PROC, true);

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
            InitSpellMap(SHADOWCRAWL_1);
        }

        void ApplyPetPassives() const
        {
            RefreshAura(MANA_LEECH);
            RefreshAura(AVOIDANCE);
        }

    private:
        uint32 liveTimer;
    };
};

void AddSC_priest_bot_pets()
{
    new priest_pet_bot();
}
