/* Copyright (C) 2006,2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../../sc_defines.h"

//Random Wispers - No txt only sound
#define RND_WISPER_1                8580 //Death is close
#define RND_WISPER_2                8581 //You are already dead
#define RND_WISPER_3                8582 //Your courage will fail
#define RND_WISPER_4                8583 //Your friends will abandon you
#define RND_WISPER_5                8584 //You will betray your friends
#define RND_WISPER_6                8585 //You will die
#define RND_WISPER_7                8586 //You are weak
#define RND_WISPER_8                8587 //Your heart will explode

//Eye Spells
#define SPELL_GREEN_BEAM                    26134
#define SPELL_DARK_GLARE                    26029
#define SPELL_RED_COLORATION                22518   //Probably not the right spell but looks similar

//Eye Tentacles Spells
#define SPELL_MIND_FLAY                     26143

//Claw Tentacles Spells
#define SPELL_GROUND_RUPTURE                26139
#define SPELL_HAMSTRING                     26141

//*****Phase 2******

//Body spells
#define SPELL_CARAPACE_CTHUN                26156

//Eye Tentacles Spells
//SAME AS PHASE1

//Giant Claw Tentacles 
#define SPELL_MASSIVE_GROUND_RUPTURE        26100
//Also casts Hamstring

//Giant Eye Tentacles
//CHAIN CASTS "SPELL_GREEN_BEAM"

//Stomach Spells
#define SPELL_EXIT_STOMACH_KNOCKBACK        26230
#define SPELL_DIGESTIVE_ACID                26476

//Mobs used for spawning
//Phase 1 (eye of c'thun)
#define BOSS_EYE_OF_CTHUN                   15589
#define MOB_CLAW_TENTACLE                   15725
#define MOB_EYE_TENTACLE                    15726

//Phase 2 (body of c'thun)
#define BOSS_CTHUN                          15727
#define MOB_GIANT_CLAW_TENTACLE             15728
#define MOB_GIANT_EYE_TENTACLE              15334
#define MOB_FLESH_TENTACLE_UKN              15802

struct MANGOS_DLL_DECL eye_of_cthunAI : public ScriptedAI
{
    eye_of_cthunAI(Creature *c) : ScriptedAI(c) {EnterEvadeMode();}

    uint32 WisperTimer; 

    uint32 PhaseTimer;
    uint32 Phase;

    //Eye beam phase
    uint32 BeamTimer;
    uint32 EyeTentacleTimer;
    uint32 ClawTentacleTimer;

    //Dark Glare phase
    uint32 RefreshTimer;
    float Angle;

    
    void EnterEvadeMode()
    {
        //One random wisper every 90 - 300 seconds
        WisperTimer = 90000;

        //Phase information
        PhaseTimer = 50000;         //
        Phase = 0;                  //0 - eye, 1 - dark, 3 - death transition

        //Eye beam phase 50 seconds
        BeamTimer = 3000;
        EyeTentacleTimer = 45000;   //Always spawns 5 seconds before Dark Beam
        ClawTentacleTimer = 12500;  //4 per Eye beam phase (unsure if they spawn durring Dark beam)

        //Dark Beam phase 35 seconds


        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
    }
    
    void AttackStart(Unit *who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack() && who!= m_creature)
            DoStartRangedAttack(who);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //We simply use this function to find players until we can use Map->GetPlayers()

        if (who && who->GetTypeId() == TYPEID_PLAYER && m_creature->IsHostileTo(who) && m_creature->IsWithinLOSInMap(who))
        {
            //Add them to our threat list
            m_creature->AddThreat(who,1.0f);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Check if we have a target
        if (!m_creature->SelectHostilTarget())
        {
            //No target so we'll use this section to do our random wispers instance wide
            //WisperTimer
            if (WisperTimer < diff)
            {
                //Play random sound to the zone
                switch (rand()%8)
                {
                    case 0:
                    DoPlaySoundToSet(m_creature,RND_WISPER_1);
                    break;
                    case 1:
                    DoPlaySoundToSet(m_creature,RND_WISPER_2);
                    break;
                    case 2:
                    DoPlaySoundToSet(m_creature,RND_WISPER_3);
                    break;
                    case 3:
                    DoPlaySoundToSet(m_creature,RND_WISPER_4);
                    break;
                    case 4:
                    DoPlaySoundToSet(m_creature,RND_WISPER_5);
                    break;
                    case 5:
                    DoPlaySoundToSet(m_creature,RND_WISPER_6);
                    break;
                    case 6:
                    DoPlaySoundToSet(m_creature,RND_WISPER_7);
                    break;
                    case 7:
                    DoPlaySoundToSet(m_creature,RND_WISPER_8);
                    break;
                }

                //One random wisper every 90 - 300 seconds
                WisperTimer = 10000;//90000 + (rand()% 210000);
            }else WisperTimer -= diff;

            return;
        }
        
        //Check if we have a current target
        if( m_creature->getVictim() && m_creature->isAlive())
        {

            //Eye Beam phase
            if (Phase == 0)
            {
                //BeamTimer
                if (BeamTimer < diff)
                {
                    //SPELL_GREEN_BEAM

                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    if (target)
                    {
                        m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
                        DoCast(target,SPELL_GREEN_BEAM);
                        //Correctly update our target
                        m_creature->SetUInt64Value(UNIT_FIELD_TARGET, target->GetGUID());
                    }

                    //Beam every 3 seconds
                    BeamTimer = 3000;
                }else BeamTimer -= diff;

                //ClawTentacleTimer
                if (ClawTentacleTimer < diff)
                {
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    if (target)
                    {
                        Unit* Spawned = NULL;

                        //Spawn claw tentacle on the random target
                        Spawned = m_creature->SummonCreature(MOB_CLAW_TENTACLE,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                        
                        if (Spawned)
                            Spawned->AddThreat(target,1.0f);
                    }

                    //One claw tentacle every 12.5 seconds
                    ClawTentacleTimer = 12500;
                }else ClawTentacleTimer -= diff;

                //EyeTentacleTimer
                if (EyeTentacleTimer < diff)
                {
                    //Spawn the 8 Eye Tentacles in the corret spots
                    Unit* target = NULL;
                    Unit* Spawned = NULL;

                    //1
                    Spawned = m_creature->SummonCreature(MOB_EYE_TENTACLE,m_creature->GetPositionX()+10,m_creature->GetPositionY()-10,m_creature->GetPositionZ(),0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);
                    if (Spawned)
                    {
                        target = SelectUnit(SELECT_TARGET_RANDOM,0);
                        if (target)
                            Spawned->AddThreat(target,1.0f);
                    }

                    //No point actually putting a timer here since
                    //These shouldn't trigger agian until after phase shifts
                    EyeTentacleTimer = 45000;
                }else EyeTentacleTimer -= diff;

                //PhaseTimer
                if (PhaseTimer < diff)
                {
                    //Switch to Dark Beam
                    Phase = 1;

                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);

                    //Select random target for dark beam to start on
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);

                    if (target)
                    {
                        //Correctly update our target
                        m_creature->SetUInt64Value(UNIT_FIELD_TARGET, target->GetGUID());

                        //Face our target
                    }

                    //Add red coloration to C'thun
                    DoCast(m_creature,SPELL_RED_COLORATION);
                    
                    //Darkbeam for 35 seconds
                    PhaseTimer = 35000;
                }else PhaseTimer -= diff;
                    
            }
            //Dark Beam phase
            else if (Phase == 1)
            {
                //EyeTentacleTimer
                if (EyeTentacleTimer < diff)
                {

                    //SPELL_DARK_GLARE
                    DoYell("Dark Beam",LANG_UNIVERSAL,NULL);

                    //
                    EyeTentacleTimer = 45000;
                }else EyeTentacleTimer -= diff;

                //PhaseTimer
                if (PhaseTimer < diff)
                {
                    //Switch to Eye Beam
                    Phase = 0;
                    BeamTimer = 3000;
                    EyeTentacleTimer = 45000;   //Always spawns 5 seconds before Dark Beam
                    ClawTentacleTimer = 12500;  //4 per Eye beam phase (unsure if they spawn durring Dark beam)

                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);

                    //Remove Red coloration from c'thun
                    m_creature->RemoveAurasDueToSpell(SPELL_RED_COLORATION);

                    //Eye Beam for 50 seconds
                    PhaseTimer = 50000;
                }else PhaseTimer -= diff;
            }
        }
    }
};

struct MANGOS_DLL_DECL eye_tentacleAI : public ScriptedAI
{
    eye_tentacleAI(Creature *c) : ScriptedAI(c) {EnterEvadeMode();}

    uint32 MindflayTimer; 
    
    void EnterEvadeMode()
    {
        //Mind flay half a second after we spawn
        MindflayTimer = 500;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
    }
    
    void AttackStart(Unit *who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack() && who!= m_creature)
            DoStartRangedAttack(who);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //We simply use this function to find players until we can use Map->GetPlayers()

        if (who && who->GetTypeId() == TYPEID_PLAYER && m_creature->IsHostileTo(who) && m_creature->IsWithinLOSInMap(who))
        {
            //Add them to our threat list
            m_creature->AddThreat(who,0.0f);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Check if we have a target
        if (!m_creature->SelectHostilTarget())
            return;
        
        //Check if we have a current target
        if( m_creature->getVictim() && m_creature->isAlive())
        {
            //MindflayTimer
            if (MindflayTimer < diff)
            {
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if (target)
                    DoCast(target,SPELL_MIND_FLAY);

                //Mindflay every 5 seconds
                MindflayTimer = 5000;
            }else MindflayTimer -= diff;
        }
    }
};

struct MANGOS_DLL_DECL claw_tentacleAI : public ScriptedAI
{
    claw_tentacleAI(Creature *c) : ScriptedAI(c) {EnterEvadeMode();}

    uint32 GroundRuptureTimer; 
    uint32 HamstringTimer;
    uint32 EvadeTimer;
    
    void EnterEvadeMode()
    {
        //First rupture should happen half a second after we spawn
        GroundRuptureTimer = 500;
        HamstringTimer = 2000;
        EvadeTimer = 5000;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
    }
    
    void AttackStart(Unit *who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack() && who!= m_creature)
            DoStartRangedAttack(who);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //We simply use this function to find players until we can use Map->GetPlayers()

        if (who && who->GetTypeId() == TYPEID_PLAYER && m_creature->IsHostileTo(who) && m_creature->IsWithinLOSInMap(who))
        {
            //Add them to our threat list
            m_creature->AddThreat(who,0.0f);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Check if we have a target
        if (!m_creature->SelectHostilTarget())
            return;
        
        //Check if we have a current target
        if( m_creature->getVictim() && m_creature->isAlive())
        {
            //EvadeTimer
            if (m_creature->GetDistanceSq(m_creature->getVictim()) > ATTACK_DISTANCE)
                if (EvadeTimer < diff)
                {
                    //Since we can't move auto evade if the players is to far away from us for to long
                    EnterEvadeMode();

                    return;
                }else EvadeTimer -= diff;

            //GroundRuptureTimer
            if (GroundRuptureTimer < diff)
            {
                DoCast(m_creature->getVictim(),SPELL_GROUND_RUPTURE);

                //Ground rupture every 30 seconds
                GroundRuptureTimer = 30000;
            }else GroundRuptureTimer -= diff;

            //HamstringTimer
            if (HamstringTimer < diff)
            {
                DoCast(m_creature->getVictim(),SPELL_HAMSTRING);

                //Hamstring every 5 seconds
                HamstringTimer = 5000;
            }else HamstringTimer -= diff;

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_eye_of_cthun(Creature *_Creature)
{
    return new eye_of_cthunAI (_Creature);
}

CreatureAI* GetAI_eye_tentacle(Creature *_Creature)
{
    return new eye_tentacleAI (_Creature);
}

CreatureAI* GetAI_claw_tentacle(Creature *_Creature)
{
    return new claw_tentacleAI (_Creature);
}

void AddSC_boss_cthun()
{
    Script *newscript;

    //Eye
    newscript = new Script;
    newscript->Name="boss_eye_of_cthun";
    newscript->GetAI = GetAI_eye_of_cthun;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_eye_tentacle";
    newscript->GetAI = GetAI_eye_tentacle;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_claw_tentacle";
    newscript->GetAI = GetAI_claw_tentacle;
    m_scripts[nrscripts++] = newscript;
}