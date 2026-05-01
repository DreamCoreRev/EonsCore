/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Cell.h"
#include "CellImpl.h"
#include "CreatureTextMgr.h"
#include "DatabaseEnv.h"
#include "GossipDef.h"
#include "GridDefines.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Language.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "SmartScript.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include "MoveSplineInit.h"
#include "GameEventMgr.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "CombatAI.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Pet.h"
#include "ScriptedEscortAI.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "World.h"
#include <cmath>

class npc_training_target : public CreatureScript
{
public:
    npc_training_target() : CreatureScript("npc_training_target") {}

    struct npc_training_targetAI : ScriptedAI
    {
        npc_training_targetAI(Creature* creature) : ScriptedAI(creature)
        {
            SetCombatMovement(false);
            entry = creature->GetEntry();
        }

        uint32 entry;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_training_targetAI(creature);
    }
};

class npc_tushui_monk : public CreatureScript
{
public:
    npc_tushui_monk() : CreatureScript("npc_tushui_monk") {}

    struct npc_tushui_monkAI : public ScriptedAI
    {
        npc_tushui_monkAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            std::list<Creature*> poleList;
            GetCreatureListWithEntryInGrid(poleList, me, 54993, 25.0f);

            if (poleList.empty())
            {
                me->DespawnOrUnsummon();
                return;
            }

            for (auto&& creature : poleList)
                me->EnterVehicle(creature);

            me->SetFaction(2357);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->ExitVehicle();
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tushui_monkAI(creature);
    }
};

// Rock Jump - 103069 / 103070 / 103077
class spell_rock_jump : public SpellScriptLoader
{
public:
    spell_rock_jump() : SpellScriptLoader("spell_rock_jump") {}

    class spell_rock_jump_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rock_jump_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->GetPositionZ() < 90.0f)
                    caster->GetMotionMaster()->MoveJump(1045.36f, 2848.47f, 91.38f, 10.0f, 10.0f, 0);
                else if (caster->GetPositionZ() < 92.0f)
                    caster->GetMotionMaster()->MoveJump(1054.42f, 2842.65f, 92.96f, 10.0f, 10.0f, 0);
                else if (caster->GetPositionZ() < 94.0f)
                    caster->GetMotionMaster()->MoveJump(1063.66f, 2843.49f, 95.50f, 10.0f, 10.0f, 0);
                else
                {
                    caster->GetMotionMaster()->MoveJump(1078.42f, 2845.07f, 95.16f, 10.0f, 10.0f, 0);

                    if (caster->ToPlayer())
                        caster->ToPlayer()->KilledMonsterCredit(57476);
                }
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_rock_jump_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_rock_jump_SpellScript();
    }
};

Position rocksPos[4] =
{
    {1102.05f, 2882.11f, 94.32f, 0.11f},
    {1120.01f, 2883.20f, 96.44f, 4.17f},
    {1128.09f, 2859.44f, 97.64f, 2.51f},
    {1111.52f, 2849.84f, 94.84f, 1.94f}
};

class npc_shu_water_spirit : public CreatureScript
{
public:
    npc_shu_water_spirit() : CreatureScript("npc_shu_water_spirit") {}

    struct npc_shu_water_spiritAI : public ScriptedAI
    {
        npc_shu_water_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap _events;
        uint8 actualPlace;
        // FIX: uint64 -> ObjectGuid
        ObjectGuid waterSpoutGUID;

        enum eShuSpells
        {
            SPELL_WATER_SPOUT_SUMMON = 116810,
            SPELL_WATER_SPOUT_WARNING = 116695,
            SPELL_WATER_SPOUT_EJECT = 116696,
            SPELL_WATER_SPOUT_VISUAL = 117057
        };

        enum eEvents
        {
            EVENT_CHANGE_PLACE = 1,
            EVENT_SUMMON_WATER_SPOUT = 2,
            EVENT_WATER_SPOUT_VISUAL = 3,
            EVENT_WATER_SPOUT_EJECT = 4,
            EVENT_WATER_SPOUT_DESPAWN = 5
        };

        void Reset() override
        {
            _events.Reset();
            actualPlace = 0;
            waterSpoutGUID.Clear();

            // FIX: ElunaTrinityWotlk uses std::chrono literals for ScheduleEvent
            _events.ScheduleEvent(EVENT_CHANGE_PLACE, 5s);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != EFFECT_MOTION_TYPE)
                return;

            if (pointId == 1)
            {
                me->RemoveAurasDueToSpell(SPELL_WATER_SPOUT_WARNING);
                // FIX: SelectNearestPlayerNotGM -> SelectNearestPlayer
                if (Player* player = me->SelectNearestPlayer(50.0f))
                {
                    // FIX: GetAngle(float x, float y) ? overload with WorldObject* not available
                    float dx = player->GetPositionX() - me->GetPositionX();
                    float dy = player->GetPositionY() - me->GetPositionY();
                    float angle = std::atan2(dy, dx);
                    me->SetOrientation(angle);
                    me->SetFacingToObject(player);
                    _events.ScheduleEvent(EVENT_SUMMON_WATER_SPOUT, 2s);
                }
                else
                    _events.ScheduleEvent(EVENT_CHANGE_PLACE, 5s);
            }
        }

        Creature* getWaterSpout()
        {
            return me->GetMap()->GetCreature(waterSpoutGUID);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
            case EVENT_CHANGE_PLACE:
            {
                uint8 newPlace = 0;

                do
                {
                    newPlace = urand(0, 3);
                } while (newPlace == actualPlace);

                me->GetMotionMaster()->MoveJump(
                    rocksPos[newPlace].GetPositionX(),
                    rocksPos[newPlace].GetPositionY(),
                    rocksPos[newPlace].GetPositionZ(),
                    10.0f, 10.0f, 1);
                me->AddAura(SPELL_WATER_SPOUT_WARNING, me);
                actualPlace = newPlace;
                break;
            }
            case EVENT_SUMMON_WATER_SPOUT:
            {
                // FIX: GetPositionWithDistInOrientation -> manual trigonometry
                float angle = me->GetOrientation() + frand(-(float)M_PI, (float)M_PI);
                float x = me->GetPositionX() + 5.0f * std::cos(angle);
                float y = me->GetPositionY() + 5.0f * std::sin(angle);

                waterSpoutGUID.Clear();

                // FIX: SummonCreature in ElunaTrinityWotlk uses Milliseconds() for despawn time
                if (Creature* waterSpout = me->SummonCreature(60488, x, y, 92.189629f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 15000ms))
                    waterSpoutGUID = waterSpout->GetGUID();

                _events.ScheduleEvent(EVENT_WATER_SPOUT_VISUAL, 500ms);
                _events.ScheduleEvent(EVENT_WATER_SPOUT_EJECT, 7500ms);
                break;
            }
            case EVENT_WATER_SPOUT_VISUAL:
            {
                if (Creature* waterSpout = getWaterSpout())
                    waterSpout->CastSpell(waterSpout, SPELL_WATER_SPOUT_WARNING, true);
                break;
            }
            case EVENT_WATER_SPOUT_EJECT:
            {
                if (Creature* waterSpout = getWaterSpout())
                {
                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, waterSpout, 1.0f);

                    for (auto&& player : playerList)
                        player->CastSpell(player, SPELL_WATER_SPOUT_EJECT, true);

                    waterSpout->CastSpell(waterSpout, SPELL_WATER_SPOUT_VISUAL, true);
                }
                _events.ScheduleEvent(EVENT_WATER_SPOUT_DESPAWN, 3s);
                break;
            }
            case EVENT_WATER_SPOUT_DESPAWN:
            {
                if (Creature* waterSpout = getWaterSpout())
                    waterSpout->DespawnOrUnsummon();

                waterSpoutGUID.Clear();

                _events.ScheduleEvent(EVENT_CHANGE_PLACE, 2s);
                break;
            }
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shu_water_spiritAI(creature);
    }
};

// Shu Benediction - 103245, 105889, 105890
class spell_shu_benediction : public AuraScript
{
    PrepareAuraScript(spell_shu_benediction);

    uint32 SelectCreature(uint32 spellId)
    {
        switch (spellId)
        {
        case 103245: return 55213;
        case 105889: return 60916;
        case 105890: return 55558;
        }
        return 0;
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        uint32 npcEntry = SelectCreature(GetSpellInfo()->Id);
        if (!npcEntry)
            return;

        std::list<Creature*> npcList;
        GetCreatureListWithEntryInGrid(npcList, target, npcEntry, 20.0f);
        for (auto&& shu : npcList)
            if (shu->ToTempSummon())
                if (shu->ToTempSummon()->GetOwnerGUID() == target->GetGUID())
                    return;

        // From here we know that the player does not have the spirit yet
        // FIX: SummonCreature with Milliseconds(0) for MANUAL_DESPAWN
        if (TempSummon* tempShu = target->SummonCreature(
            npcEntry,
            target->GetPositionX(),
            target->GetPositionY(),
            target->GetPositionZ(),
            0.0f,
            TEMPSUMMON_MANUAL_DESPAWN,
            0ms))
        {
            // FIX: SetExplicitSeerGuid removed ? does not exist in ElunaTrinityWotlk
            tempShu->SetOwnerGUID(target->GetGUID());
            tempShu->GetMotionMaster()->MoveFollow(target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        uint32 npcEntry = SelectCreature(GetSpellInfo()->Id);
        if (!npcEntry)
            return;

        std::list<Creature*> npcList;
        GetCreatureListWithEntryInGrid(npcList, target, npcEntry, 20.0f);
        for (auto&& shu : npcList)
            if (shu->ToTempSummon())
                if (shu->ToTempSummon()->GetOwnerGUID() == target->GetGUID())
                    shu->DespawnOrUnsummon();
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_shu_benediction::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectRemove += AuraEffectRemoveFn(spell_shu_benediction::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

void AddSC_zone_the_wandering_isle()
{
    new npc_training_target();
    new npc_tushui_monk();
    new spell_rock_jump();
    new npc_shu_water_spirit();
    new spell_shu_benediction();
}
