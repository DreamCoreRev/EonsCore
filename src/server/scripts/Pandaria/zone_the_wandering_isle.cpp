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

class npc_shang_xi_air_balloon : public CreatureScript
{
public:
    npc_shang_xi_air_balloon() : CreatureScript("npc_shang_xi_air_balloon") {}

    struct npc_shang_xi_air_balloonAI : public CreatureAI
    {
        npc_shang_xi_air_balloonAI(Creature* creature) : CreatureAI(creature) {}

        uint64 playerGUID;
        uint32 eventTimer;
        uint32 phase;

        void Reset() override
        {
            playerGUID = 0;
            eventTimer = 250;
            phase = 0;

            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
        }

        void RemoveNpcPassengers()
        {
            for (auto i = 1; i != 3; ++i)
            {
                auto const passenger = me->GetVehicleKit()->GetPassenger(i);
                if (!passenger)
                    continue;

                passenger->ExitVehicle();
                passenger->ToCreature()->DespawnOrUnsummon(1000s);
            }
        }

        void WaypointReached(uint32 /*nodeId*/, uint32 /*pathId*/)
        {
            
        }

        /*
            seat 0 = player
            seat 1 = Ji Firepaw
            seat 2 = Aysa Cloudsinger
        */

        void UpdateAI(uint32 /*diff*/) override
        {
            if (playerGUID == 0)
            {
                RemoveNpcPassengers();
                me->DespawnOrUnsummon();
                return;
            }
        }

        void PassengerTalk(uint32 /*talkId*/, uint32 /*seatId*/)
        {
            
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || passenger->GetTypeId() != TYPEID_PLAYER)
                return;

            if (apply)
            {
                auto const player = passenger->ToPlayer();

                if (auto const firepaw = player->FindNearestCreature(56660, player->GetPositionX()))
                    firepaw->EnterVehicle(me, 1);

                if (auto const aysa = player->FindNearestCreature(56662, player->GetPositionX()))
                    aysa->EnterVehicle(me, 2);

                player->KilledMonsterCredit(56378);
            }
            else
                playerGUID = 0;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shang_xi_air_balloonAI(creature);
    }
};

class mob_aisa_pre_balon_event : public CreatureScript
{
public:
    mob_aisa_pre_balon_event() : CreatureScript("mob_aisa_pre_balon_event") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 29791)
            sCreatureTextMgr->SendChat(creature, 1);

        creature->CastSpell(player, 95247, true);
        player->CastSpell(player, 95247, true);
        return true;
    }

    struct mob_aisa_pre_balon_eventAI : public ScriptedAI
    {
        mob_aisa_pre_balon_eventAI(Creature* creature) : ScriptedAI(creature) {}

        bool justSpeaking;
        EventMap _events;
        GuidSet m_player_for_event;

        enum events
        {
            EVENT_1 = 1,
            EVENT_2 = 2,
            EVENT_3 = 3,

            NPC_FRIEND = 56663,
        };

        void Reset() override
        {
            justSpeaking = false;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (justSpeaking || who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle(0))
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());

            if (itr != m_player_for_event.end())
                return;

            if (who->ToPlayer()->GetQuestStatus(29790) != QUEST_STATUS_COMPLETE)
                return;

            m_player_for_event.insert(who->GetGUID());
            justSpeaking = true;
            _events.ScheduleEvent(EVENT_1, 10000s);
            sCreatureTextMgr->SendChat(me, 0);

        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                {
                    _events.ScheduleEvent(EVENT_2, 8000s);

                    if (Creature* f = me->FindNearestCreature(NPC_FRIEND, 100.0f, true))
                    {
                        sCreatureTextMgr->SendChat(f, 0);
                        f->SetFacingToObject(me);
                    }
                    break;
                }
                case EVENT_2:
                    sCreatureTextMgr->SendChat(me, 2);
                    justSpeaking = false;
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_aisa_pre_balon_eventAI(creature);
    }
};

class mop_air_balloon : public CreatureScript
{
public:
    mop_air_balloon() : CreatureScript("mop_air_balloon") {}

    struct mop_air_balloonAI : public CreatureAI
    {
        mop_air_balloonAI(Creature* creature) : CreatureAI(creature) {}

        ObjectGuid playerGuid;
        ObjectGuid aisaGUID;
        ObjectGuid firepawGUID;
        ObjectGuid shenZiGUID;
        ObjectGuid headGUID;
        EventMap events;

        void Reset() override
        {
            me->SetWalk(false);
            me->SetSpeed(MOVE_FLIGHT, 10.0f);
            events.Reset();

            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
            me->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK5);
            me->m_invisibilityDetect.AddValue(INVISIBILITY_UNK5, 999);
        }

        enum localdata
        {
            NPC_AISA = 56661,
            NPC_FIREPAW = 56660,
            NPC_SHEN_ZI_SU = 56676,
            NPC_TURTLE_HEAD = 57769,

            SPELL_HEAD_ANIM_RISE = 114888,
            SPELL_HEAD_ANIM_1 = 114898,
            SPELL_HEAD_ANIM_2 = 118571,
            SPELL_HEAD_ANIM_3 = 118572,
            SPELL_VOICE_ANIM = 106759,

            SPELL_AISA_ENTER_SEAT_2 = 63313, //106617

            SPELL_CREDIT_1 = 105895,
            SPELL_CREDIT_2 = 105010,
            SPELL_EJECT_PASSANGER = 60603,
            SPELL_PARASHUT = 45472,

            EVENT_1 = 1, // 17:24:47.000

            EVENT_AISA_TALK_0 = 2,  //17:24:51.000
            EVENT_AISA_TALK_1 = 3,  //17:25:07.000
            EVENT_AISA_TALK_2 = 4,  //17:25:18.000
            EVENT_AISA_TALK_3 = 5,  //17:25:31.000
            EVENT_AISA_TALK_4 = 6,  //17:25:38.000
            EVENT_AISA_TALK_5 = 7,  //17:26:40.000
            EVENT_AISA_TALK_6 = 8,  //17:27:02.000
            EVENT_AISA_TALK_7 = 9,  //17:27:29.000
            EVENT_AISA_TALK_8 = 10, //17:27:50.000
            EVENT_AISA_TALK_9 = 11, //17:28:04.000
            EVENT_AISA_TALK_10 = 12, //17:28:10.000

            EVENT_FIREPAW_TALK_0 = 13, //17:24:47.000
            EVENT_FIREPAW_TALK_1 = 14, //17:24:57.000
            EVENT_FIREPAW_TALK_2 = 15, //17:25:13.000
            EVENT_FIREPAW_TALK_3 = 16, //17:27:16.000
            EVENT_FIREPAW_TALK_4 = 17, //17:27:22.000
            EVENT_FIREPAW_TALK_5 = 18, //17:27:43.000
            EVENT_FIREPAW_TALK_6 = 19, //17:27:57.000

            EVENT_SHEN_ZI_SU_TALK_0 = 20, //17:25:44.000
            EVENT_SHEN_ZI_SU_TALK_1 = 21, //17:25:58.000
            EVENT_SHEN_ZI_SU_TALK_2 = 22, //17:26:12.000
            EVENT_SHEN_ZI_SU_TALK_3 = 23, //17:26:25.000
            EVENT_SHEN_ZI_SU_TALK_4 = 24, //17:26:47.000 
            EVENT_SHEN_ZI_SU_TALK_5 = 25, //17:27:09.000
        };

        void InitTalking(Player* /*player*/)
        {
            
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply)
        {
            if (!apply)
            {
                if (passenger->GetTypeId() == TYPEID_PLAYER)
                {
                    me->DespawnOrUnsummon(1000s);
                    me->CastSpell(passenger, 45472, true);
                    passenger->ToPlayer()->KilledMonsterCredit(55939);
                }
                else
                    passenger->ToCreature()->DespawnOrUnsummon(1000s);
                return;
            }

            if (seatId == 0)
            {
                if (Player* player = passenger->ToPlayer())
                {
                    playerGuid = player->GetGUID();
                    me->CastSpell(player, 105895, true);
                    InitTalking(player);
                }
            }

            if (passenger->GetTypeId() != TYPEID_PLAYER)
            {
                passenger->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK5);
                passenger->m_invisibilityDetect.AddValue(INVISIBILITY_UNK5, 999);

                switch (passenger->GetEntry())
                {
                case 56661:
                    aisaGUID = passenger->GetGUID();
                    break;
                case 56660:
                    firepawGUID = passenger->GetGUID();
                    break;
                default:
                    break;
                }
            }
        }

        void TalkShenZiSU(uint32 text)
        {
            Creature* shen = me->GetMap()->GetCreature(shenZiGUID);

            if (!shen)
                return;

            if (Player* plr = ObjectAccessor::FindPlayer(playerGuid))
            {
                Creature* head = me->GetMap()->GetCreature(headGUID);
                if (!head)
                    return;

                switch (text)
                {
                    //cast 114888                  //17:25:31.000
                case 0:                            //17:25:44.000
                    plr->CastSpell(shen, SPELL_HEAD_ANIM_1, false);
                    break;
                case 1:                            //17:25:58.000
                    plr->CastSpell(shen, SPELL_VOICE_ANIM, false);
                    break;
                case 2:                            //17:26:11.000
                case 3:                            //17:26:25.000
                case 5:                            //17:27:08.000
                    plr->CastSpell(shen, SPELL_HEAD_ANIM_2, false);
                    break;
                case 4:                            //17:26:47.000
                    plr->CastSpell(shen, SPELL_HEAD_ANIM_3, false);
                    break;
                }
                if (text == 5) // restore emote
                {
                    head->SetUInt32Value(UNIT_NPC_EMOTESTATE, ANIM_FLY_LAND);
                }
            }
            sCreatureTextMgr->SendChat(shen, text);
        }

        void IsSummonedBy(WorldObject* /*summoner*/)
        {
           
        }

        void WaypointReached(uint32 /*nodeId*/, uint32 /*pathId*/)
        {
           
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                {
                    if (Creature* f = me->FindNearestCreature(NPC_AISA, 100.0f, true))
                        f->CastSpell(me, SPELL_AISA_ENTER_SEAT_2, true);

                    break;
                }
                case EVENT_AISA_TALK_3:
                    if (Creature* head = me->GetMap()->GetCreature(headGUID))
                        if (Player* plr = ObjectAccessor::FindPlayer(playerGuid))
                        {
                            plr->CastSpell(plr, SPELL_HEAD_ANIM_RISE, false);    //17:25:31.000
                            head->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                        }
                case EVENT_AISA_TALK_0:
                case EVENT_AISA_TALK_1:
                case EVENT_AISA_TALK_2:
                case EVENT_AISA_TALK_4:
                case EVENT_AISA_TALK_5:
                case EVENT_AISA_TALK_6:
                case EVENT_AISA_TALK_7:
                case EVENT_AISA_TALK_8:
                case EVENT_AISA_TALK_9:
                case EVENT_AISA_TALK_10:
                {
                    if (Creature* aisa = me->GetMap()->GetCreature(aisaGUID))
                        sCreatureTextMgr->SendChat(aisa, eventId - 2, 0);
                    break;
                }

                case EVENT_FIREPAW_TALK_0:
                case EVENT_FIREPAW_TALK_1:
                case EVENT_FIREPAW_TALK_2:
                case EVENT_FIREPAW_TALK_3:
                case EVENT_FIREPAW_TALK_4:
                case EVENT_FIREPAW_TALK_5:
                case EVENT_FIREPAW_TALK_6:
                {
                    if (Creature* paw = me->GetMap()->GetCreature(firepawGUID))
                        sCreatureTextMgr->SendChat(paw, eventId - 13, 0);
                    break;
                }
                case EVENT_SHEN_ZI_SU_TALK_0:   //114898
                case EVENT_SHEN_ZI_SU_TALK_1:   //cast 106759
                case EVENT_SHEN_ZI_SU_TALK_2:   //cast 118571
                case EVENT_SHEN_ZI_SU_TALK_3:   //118571
                case EVENT_SHEN_ZI_SU_TALK_4:   //118572
                case EVENT_SHEN_ZI_SU_TALK_5:   //118571
                    TalkShenZiSU(eventId - 20);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mop_air_balloonAI(creature);
    }
};

void AddSC_zone_the_wandering_isle()
{
    new npc_training_target();
    new npc_tushui_monk();
    new spell_rock_jump();
    new npc_shang_xi_air_balloon();
    new mob_aisa_pre_balon_event();
    new mop_air_balloon();
}
