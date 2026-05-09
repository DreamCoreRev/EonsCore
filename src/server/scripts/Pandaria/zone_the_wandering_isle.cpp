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
#include "EventMap.h"
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

class npc_training_target : public CreatureScript
{
public:
    npc_training_target() : CreatureScript("npc_training_target") { }

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

enum TushuiEvents
{
    EVENT_ENTER_VEHICLE = 1
};

class npc_tushui_monk : public CreatureScript
{
public:
    npc_tushui_monk() : CreatureScript("npc_tushui_monk") {}

    struct npc_tushui_monkAI : public ScriptedAI
    {
        npc_tushui_monkAI(Creature* creature) : ScriptedAI(creature), _boarded(false) {}

        EventMap _events;
        bool _boarded;

        void Reset() override
        {
            _boarded = false;
            me->SetFaction(2357);

            _events.ScheduleEvent(EVENT_ENTER_VEHICLE, 500ms);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENTER_VEHICLE:
                    {
                        if (me->GetVehicle() || _boarded)
                            return;

                        std::list<Creature*> poleList;
                        GetCreatureListWithEntryInGrid(poleList, me, 54993, 25.0f);

                        if (poleList.empty())
                        {
                            me->DespawnOrUnsummon(1000ms);
                            return;
                        }

                        for (Creature* pole : poleList)
                        {
                            if (Vehicle* vk = pole->GetVehicleKit())
                            {
                                if (vk->GetAvailableSeatCount() > 0)
                                {
                                    me->EnterVehicle(pole);
                                    _boarded = true;
                                    break;
                                }
                            }
                        }

                        if (!_boarded)
                            me->DespawnOrUnsummon(1000ms);

                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _events.Reset();
            me->ExitVehicle();
            me->DespawnOrUnsummon(1000ms);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tushui_monkAI(creature);
    }
};

// Rock Jump - 103069 / 103070 / 103077
class spell_rock_jump: public SpellScriptLoader
{
    public:
        spell_rock_jump() : SpellScriptLoader("spell_rock_jump") { }

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

void AddSC_zone_the_wandering_isle()
{
    new npc_training_target();
    new npc_tushui_monk();
    new spell_rock_jump();
}
